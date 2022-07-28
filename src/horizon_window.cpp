#include "horizon_window.h"

#include "debug.h"

namespace horizon {

Window::Window(int width, int height, std::string name) :
    mWidth(width), mHeight(height), mName(name) {
    if (!glfwInit()) {
        throw std::runtime_error("GLFW failed to initialize!");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), NULL, NULL);
    if (!mWindow) {
        throw std::runtime_error("Failed to create glfw window!");
    }
}

Window::~Window() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

} // namespace horizon
