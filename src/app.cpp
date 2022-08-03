#include "app.h"

#include "render_systems/horizon_imgui_render_system.h"
#include "render_systems/horizon_model_renderer.h"
#include "render_systems/horizon_point_light_renderer.h"
#include "core/horizon_components.h"
#include "gfx/horizon_descriptor.h"
#include "core/ecs.h"

App* App::init() {
    App *app = new App();

    return app;
}

void App::shutdown(App *application) {
    delete application;
}

void App::run() {
    std::unique_ptr<horizon::gfx::DescriptorPool> globalPool = horizon::gfx::DescriptorPool::Builder(device)
                                                                   .setMaxSets(horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT)
                                                                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT)
                                                                   .build();
                                                
    std::vector<std::unique_ptr<horizon::gfx::Buffer>> uboBuffers;
    for (int i = 0; i < horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        uboBuffers.push_back(std::make_unique<horizon::gfx::Buffer>(device,
                                                                    sizeof(horizon::UBO),
                                                                    1,
                                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        uboBuffers[i]->map();
    }

    std::unique_ptr<horizon::gfx::DescriptorSetLayout> globalSetLayout = horizon::gfx::DescriptorSetLayout::Builder(device)
                                                                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                                                             .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->getDescriptorInfo();
        horizon::gfx::DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .pushWrites(globalDescriptorSets[i]);
    }

    // horizon::TestRenderCamera test{device, renderer.getSwapChainRenderPass(), globalSetLayout->getSetLayout()};
    horizon::ModelRenderer modelRenderer{device, renderer, globalSetLayout->getSetLayout()}; 
    horizon::ImGuiRenderSystem imguiRenderSystem{window, device, renderer};
    horizon::PointLightRenderer pointLigthRenderer{device, renderer, globalSetLayout->getSetLayout()};

    ecs::Scene scene;
    
    auto cameraEnt = scene.newEntity();
    auto [camera, cameraTransform] = scene.assign<horizon::Camera, horizon::Transform>(cameraEnt);
    camera.setPerspectiveProjection(90.0f, window.getAspect(), 0.001f, 100.0f);
    cameraTransform.translation.z = -2.5;

    auto ent = scene.newEntity();
    auto [transform, model] = scene.assign<horizon::Transform, horizon::Model>(ent);
    model.init(device, "../assets/flat_vase.obj");
    transform.translation = {0, 0, 0};

    auto pointLightEnt = scene.newEntity();
    auto& pointLight = scene.assign<horizon::PointLight>(pointLightEnt);
    pointLight.position = {-1, 0, -1, 0};
    pointLight.color = {1, 1, 1, 1};
    pointLight.radius = .05;

    horizon::Controller controller{};

    horizon::UBO ubo = {};

    while (!window.shouldClose()) {
        window.pollEvents();
        
        if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_L) != GLFW_PRESS) {
            controller.moveInPlaneXZ(window.getGLFWwindow(), 0.01, cameraTransform.translation, cameraTransform.rotation);
            camera.setViewYXZ(cameraTransform.translation, cameraTransform.rotation);
        } else {
            horizon::Transform transform;
            transform.translation = pointLight.position;
            controller.moveInPlaneXZ(window.getGLFWwindow(), 0.01, transform.translation, transform.rotation);
            pointLight.position = glm::vec4{transform.translation, 1.0f};
        }

        // update
        ubo.PV = camera.getProjection() * camera.getView();
        ubo.cameraPos = cameraTransform.translation;
        int i = 0;
        for (auto [ent, pointLight] : ecs::SceneView<horizon::PointLight>(scene)) {
            ubo.pointLights[i] = pointLight;
            if (i > MAX_POINT_LIGHTS) {
                break;
            }
            i++;
        }
        ubo.numPointLights = i;

        imguiRenderSystem.newFrame();

        // rendering
        if (auto commandBuffer = renderer.beginFrame()) {
            int frameIndex = renderer.getFrameIndex();
            
            // update ubo
            uboBuffers[frameIndex]->writeToMappedBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            renderer.beginSwapChainRenderPass(commandBuffer);
            modelRenderer.render(commandBuffer, globalDescriptorSets[frameIndex], scene);
            pointLigthRenderer.render(commandBuffer, globalDescriptorSets[frameIndex], scene);
            imguiRenderSystem.render(commandBuffer);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    device.waitIdle();
    model.free();
}

