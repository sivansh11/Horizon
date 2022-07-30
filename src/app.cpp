#include "app.h"

#include "render_systems/horizon_test_triangle_2.h"

App* App::init() {
    App *app = new App();

    return app;
}

void App::shutdown(App *application) {
    delete application;
}

void App::run() {
    horizon::TestTriangle2 test{device, renderer.getSwapChainRenderPass()};

    while (!window.shouldClose()) {
        window.pollEvents();
        
        // rendering
        if (auto commandBuffer = renderer.beginFrame()) {
            renderer.beginSwapChainRenderPass(commandBuffer);
            test.render(commandBuffer);
            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }
    device.waitIdle();
}

