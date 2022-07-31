#include "app.h"

#include "render_systems/horizon_test_render_camera.h"
#include "core/horizon_camera.h"
#include "core/horizon_transform.h"
#include "core/horizon_mesh.h"
#include "core/horizon_controller.h"
#include "core/ecs.h"

App* App::init() {
    App *app = new App();

    return app;
}

void App::shutdown(App *application) {
    delete application;
}

std::vector<horizon::Vertex> vertices {
    {{-1.0f,-1.0f,-1.0f}, {1, 0, 0}, {1, 1, 1}, {0, 0}}, // triangle 1 : begin
    {{-1.0f,-1.0f, 1.0f}, {0, 1, 0}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, {0, 0, 1}, {1, 1, 1}, {0, 0}}, // triangle 1 : end
    {{ 1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}}, // triangle 2 : begin
    {{-1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}}, // triangle 2 : end
    {{ 1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f,-1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{-1.0f, 1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}},
    {{ 1.0f,-1.0f, 1.0f}, {1, 1, 1}, {1, 1, 1}, {0, 0}}
};

void App::run() {
    horizon::TestRenderCamera test{device, renderer.getSwapChainRenderPass()};

    ecs::Scene scene;

    horizon::Controller controller{};
    
    auto cameraEnt = scene.newEntity();
    auto [camera, cameraTransform] = scene.assign<horizon::Camera, horizon::Transform>(cameraEnt);
    camera.setPerspectiveProjection(90.0f, window.getAspect(), 0.001f, 100.0f);

    auto ent = scene.newEntity();
    auto [transform, mesh] = scene.assign<horizon::Transform, horizon::Mesh>(ent);
    mesh.init(device, &vertices, nullptr);
    transform.translation = {0, 0, 0};


    while (!window.shouldClose()) {
        window.pollEvents();
        
        controller.moveInPlaneXZ(window.getGLFWwindow(), 0.01, cameraTransform.translation, cameraTransform.rotation);
        camera.setViewYXZ(cameraTransform.translation, cameraTransform.rotation);
        
        // rendering
        if (auto commandBuffer = renderer.beginFrame()) {
            renderer.beginSwapChainRenderPass(commandBuffer);
            test.render(commandBuffer, scene, cameraEnt);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    device.waitIdle();
    mesh.free();
}

