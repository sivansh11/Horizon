#include "app.h"

#include "render_systems/horizon_test_render_camera.h"
#include "render_systems/horizon_imgui_render_system.h"
#include "core/horizon_camera.h"
#include "core/horizon_transform.h"
#include "core/horizon_mesh.h"
#include "core/horizon_controller.h"
#include "gfx/horizon_descriptor.h"
#include "core/ecs.h"

App* App::init() {
    App *app = new App();

    return app;
}

void App::shutdown(App *application) {
    delete application;
}

inline constexpr const glm::vec3 red    = {1, 0, 0};
inline constexpr const glm::vec3 yellow = {1, 1, 0};
inline constexpr const glm::vec3 white  = {1, 1, 1};
inline constexpr const glm::vec3 green  = {0, 1, 0};
inline constexpr const glm::vec3 purple = {1, 0, 1};
inline constexpr const glm::vec3 blue   = {0, 0, 1};

std::vector<horizon::Vertex> vertices {
    {{-1.0f,-1.0f,-1.0f}, red    , {1, 1, 1}, {0, 0}}, // triangle 1 : begin
    {{-1.0f,-1.0f, 1.0f}, red    , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, red    , {1, 1, 1}, {0, 0}}, // triangle 1 : end
    {{ 1.0f, 1.0f,-1.0f}, red    , {1, 1, 1}, {0, 0}}, // triangle 2 : begin
    {{-1.0f,-1.0f,-1.0f}, red    , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, red    , {1, 1, 1}, {0, 0}}, // triangle 2 : end
    {{ 1.0f,-1.0f, 1.0f}, yellow , {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, yellow , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, yellow , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f,-1.0f}, yellow , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, yellow , {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, yellow , {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, white  , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, white  , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, white  , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, white  , {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f, 1.0f}, white  , {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, white  , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, green  , {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f, 1.0f}, green  , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, green  , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, green  , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, green  , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f,-1.0f}, green  , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, purple , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, purple , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, purple , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, purple , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f,-1.0f}, purple , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, purple , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, blue   , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, blue   , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, blue   , {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, blue   , {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, blue   , {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, blue   , {1, 1, 1}, {0, 0}}
};

void App::run() {
    std::unique_ptr<horizon::gfx::DescriptorPool> globalPool = horizon::gfx::DescriptorPool::Builder(device)
                                                                   .setMaxSets(horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT)
                                                                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT)
                                                                   .build();
                                                
    std::vector<std::unique_ptr<horizon::gfx::Buffer>> uboBuffers;
    for (int i = 0; i < horizon::gfx::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        uboBuffers.push_back(std::make_unique<horizon::gfx::Buffer>(device,
                                                                    sizeof(horizon::Ubo),
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

    horizon::TestRenderCamera test{device, renderer.getSwapChainRenderPass(), globalSetLayout->getSetLayout()};

    ecs::Scene scene;
    
    auto cameraEnt = scene.newEntity();
    auto [camera, cameraTransform] = scene.assign<horizon::Camera, horizon::Transform>(cameraEnt);
    camera.setPerspectiveProjection(90.0f, window.getAspect(), 0.001f, 100.0f);

    auto ent = scene.newEntity();
    auto [transform, mesh] = scene.assign<horizon::Transform, horizon::Mesh>(ent);
    mesh.init(device, &vertices, nullptr);
    transform.translation = {0, 0, 0};

    horizon::Controller controller{};

    horizon::ImGuiRenderSystem imguiRenderSystem{window, device, renderer, static_cast<uint32_t>(renderer.getSwapChainImageCount())};

    while (!window.shouldClose()) {
        window.pollEvents();
        
        controller.moveInPlaneXZ(window.getGLFWwindow(), 0.01, cameraTransform.translation, cameraTransform.rotation);
        camera.setViewYXZ(cameraTransform.translation, cameraTransform.rotation);

        imguiRenderSystem.newFrame();

        ImGui::Begin("test window");
        ImGui::Text("hello!");
        ImGui::End();

        // rendering
        if (auto commandBuffer = renderer.beginFrame()) {
            int frameIndex = renderer.getFrameIndex();
            horizon::Ubo ubo = {camera.getProjection() * camera.getView()};
            uboBuffers[frameIndex]->writeToMappedBuffer(&ubo);
            uboBuffers[frameIndex]->flush();
            renderer.beginSwapChainRenderPass(commandBuffer);
            test.render(commandBuffer, globalDescriptorSets[frameIndex], scene, cameraEnt);
            imguiRenderSystem.render(commandBuffer);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    device.waitIdle();
    mesh.free();
}

