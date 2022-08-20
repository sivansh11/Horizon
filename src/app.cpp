#include "app.h"

#include "render_systems/horizon_test_texture_renderer.h"
#include "render_systems/horizon_imgui_render_system.h"
#include "gfx/horizon_texture.h"
#include "gfx/horizon_buffer.h"
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
                                                                   .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT)
                                                                   .build();
                                                
    std::unique_ptr<horizon::gfx::DescriptorSetLayout> globalSetLayout = horizon::gfx::DescriptorSetLayout::Builder(device)
                                                                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                                                             .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                                                             .build();

    std::vector<std::unique_ptr<horizon::gfx::Buffer>> uboBuffers(horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<horizon::gfx::Buffer>(
            device,
            sizeof(horizon::UBO),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffers[i]->map();
    }

    horizon::gfx::Texture2D tex(device, "../assets/textures/936378.jpg");
    horizon::gfx::Texture2D tex2(device, "../assets/textures/example.jpeg");

    std::vector<VkDescriptorSet> globalDescriptorSets(horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->getDescriptorInfo();
        auto imageInfo = tex.getDescriptorInfo();
        horizon::gfx::DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &imageInfo)
            .pushWrites(globalDescriptorSets[i]);
    }

    horizon::TestTextureRenderer testRenderer(device, renderer, globalSetLayout->getSetLayout());
    horizon::ImGuiRenderSystem imguiRenderer(window, device, renderer);

    ecs::Scene scene;

    auto cameraEnt = scene.newEntity();
    auto [camera, cameraTransform] = scene.assign<horizon::Camera, horizon::Transform>(cameraEnt);
    camera.setPerspectiveProjection(60.0f, window.getAspect(), 0.01, 100);

    auto ent = scene.newEntity();
    auto [model, transform] = scene.assign<horizon::Model, horizon::Transform>(ent);
    model.init(device, "../assets/quad.obj");

    horizon::Controller controller;
    
    while (!window.shouldClose()) {
        window.pollEvents();

        controller.moveInPlaneXZ(window.getGLFWwindow(), 0.01, cameraTransform.translation, cameraTransform.rotation);
        camera.setViewYXZ(cameraTransform.translation, cameraTransform.rotation);

        if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_C) == GLFW_PRESS) {
            globalPool->resetPool();
            for (int i = 0; i < globalDescriptorSets.size(); i++) {
                auto bufferInfo = uboBuffers[i]->getDescriptorInfo();
                auto imageInfo = tex2.getDescriptorInfo();
                horizon::gfx::DescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo)
                    .pushWrites(globalDescriptorSets[i]);
            }
        } else {
            globalPool->resetPool();
            for (int i = 0; i < globalDescriptorSets.size(); i++) {
                auto bufferInfo = uboBuffers[i]->getDescriptorInfo();
                auto imageInfo = tex.getDescriptorInfo();
                horizon::gfx::DescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo)
                    .pushWrites(globalDescriptorSets[i]);
            }
        }

        imguiRenderer.newFrame();
        ImGui::Text("Hello!");
        
        // rendering
        if (auto commandBuffer = renderer.beginFrame()) {
            int frameIndex = renderer.getFrameIndex();
            
            horizon::UBO ubo{};
            ubo.PV = camera.getProjection() * camera.getView();
            
            // update ubo
            uboBuffers[frameIndex]->writeToMappedBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            renderer.beginSwapChainRenderPass(commandBuffer);
            
            testRenderer.render(commandBuffer, globalDescriptorSets[frameIndex], scene);
            imguiRenderer.render(commandBuffer);

            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    device.waitIdle();
    model.free();
}

