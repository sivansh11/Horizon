#include "app.h"

#include "gfx/render_systems/horizon_test_triangle.h"
#include "gfx/core/horizon_buffer.h"

App* App::init() {
    App *app = new App();

    return app;
}

void App::shutdown(App *application) {
    delete application;
}

struct ubo { int i; };

void App::run() {
    horizon::gfx::TestTriangle test{device, renderer.getSwapChainRenderPass()};

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

