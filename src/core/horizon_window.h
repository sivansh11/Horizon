#ifndef CORE_HORIZON_WINDOW_H
#define CORE_HORIZON_WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace horizon {

class Window {
public:
    Window(int width, int height, std::string name);
    ~Window();

    bool shouldClose() { return glfwWindowShouldClose(mWindow); }

    VkExtent2D getExtent() { return {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight)}; }
    void pollEvents() { return glfwPollEvents(); }
    GLFWwindow *getGLFWwindow() { return mWindow; }
    VkResult createWindowSurface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) {
        return glfwCreateWindowSurface(instance, mWindow, allocator, surface);
    }

    float getAspect() { return static_cast<float>(mWidth) / static_cast<float>(mHeight); }

private:
    GLFWwindow *mWindow;
    int mWidth, mHeight;
    std::string mName;
};

} // namespace horizon

#endif