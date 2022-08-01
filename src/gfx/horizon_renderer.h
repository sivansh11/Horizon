#ifndef GFX_HORIZON_RENDERER_H
#define GFX_HORIZON_RENDERER_H

#include "core/debug.h"

#include "core/horizon_window.h"
#include "gfx/horizon_device.h"
#include "gfx/horizon_swapchain.h"

#include <memory>

namespace horizon {

namespace gfx {

class Renderer {
public:
    Renderer(Window &window, Device &device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    void operator=(const Renderer&) = delete;
    Renderer(const Renderer&&) = delete;
    void operator=(const Renderer&&) = delete;

    VkCommandBuffer beginFrame();
    void endFrame();

    // for presenting to the window surface
    void beginSwapChainRenderPass(VkCommandBuffer commandBuffer); 
    void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    VkCommandBuffer getCurrentCommandBuffer() {
        ASSERT(isFrameStarted, "Cannot get command buffer when frame not in progress!");
        return mCommandBuffers[currentFrameIndex];
    }

    int getFrameIndex() {
        ASSERT(isFrameStarted, "Cannot get frame index if frame not in progress!");
        return currentFrameIndex;
    }

    size_t getSwapChainImageCount() { return mSwapChain->getImageCount(); }
    VkSurfaceFormatKHR& getSwapChainSurfaceFormat() { return mSwapChain->getSurfaceFormat(); }
    VkRenderPass& getSwapChainRenderPass() { return mSwapChain->getRenderPass(); }
    VkPresentModeKHR& getSwapChainPresentMode() { return mSwapChain->getPresentMode(); }
    VkSwapchainKHR& getSwapChain() { return mSwapChain->getSwapChain(); }

private:
    void recreateSwapChain();
    void createCommandBuffers();

private:
    Window& mWindow;
    Device& mDevice;

    std::unique_ptr<SwapChain> mSwapChain{nullptr};
    std::vector<VkCommandBuffer> mCommandBuffers{};

    VkClearValue clearValues = {{{0.01f, 0.01f, 0.01f, 1.0f}}};

    bool isFrameStarted = false;

    uint32_t currentImageIndex;
    int currentFrameIndex = 0;
};

} // namespace gfx

} // namespace horizon

#endif