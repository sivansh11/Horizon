#ifndef APP_H
#define APP_H

#include "core/horizon_window.h"
#include "gfx/horizon_device.h"
#include "gfx/horizon_renderer.h"

class App {
public:
    App() = default;
    
    static App* init();
    static void shutdown(App *application);
    void run();

private:
    horizon::Window window{800, 600, "Horizon"};
    horizon::gfx::Device device{window};
    horizon::gfx::Renderer renderer{window, device};
};

#endif