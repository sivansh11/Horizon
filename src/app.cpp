#include "app.h"

#include "render_systems/horizon_test_render_camera.h"
#include "core/horizon_camera.h"
#include "core/horizon_transform.h"
#include "core/ecs.h"
#include "core/horizon_mesh.h"

App* App::init() {
    App *app = new App();

    return app;
}

void App::shutdown(App *application) {
    delete application;
}

std::vector<horizon::Vertex> vertices {
    {{-.5, .5, 0}, {1.0, 0, 0}, {0, 0, 0}, {0, 0}},
    {{.5, .5, 0}, {0, 1.0, 0}, {0, 0, 0}, {0, 0}},
    {{0, -.5, 0}, {0, 0, 1.0}, {0, 0, 0}, {0, 0}}
};

void App::run() {
    horizon::TestRenderCamera test{device, renderer.getSwapChainRenderPass()};

    ecs::Scene scene;
    
    auto cameraEnt = scene.newEntity();
    auto& camera = scene.assign<horizon::Camera>(cameraEnt);
    camera.setPerspectiveProjection(90.0f, window.getAspect(), 0.001f, 100.0f);
    camera.setViewTarget({0, 0, 1}, {0, 0, 0});

    auto ent = scene.newEntity();
    auto [transform, mesh] = scene.assign<horizon::Transform, horizon::Mesh>(ent);
    mesh.init(device, &vertices, nullptr);
    transform.translation = {0, 0, 0};


    while (!window.shouldClose()) {
        window.pollEvents();
        
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

