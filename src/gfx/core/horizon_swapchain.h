#ifndef GFX_CORE_HORIZON_SWAPCHAIN_H
#define GFX_CORE_HORIZON_SWAPCHAIN_H

#include "gfx/core/horizon_device.h"

#include <memory>

namespace horizon {
    
namespace gfx {
    
class SwapChain {
public:
    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    SwapChain(VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();

    bool operator==(const SwapChain &other) {
        return mSwapChainFormat == other.mSwapChainFormat;
    }
    
    bool operator!=(const SwapChain &other) {
        return !operator==(other);
    }

    VkResult acquireNextImage(uint32_t &imageIndex);
    VkResult submitCommandBuffer(VkCommandBuffer &buffer, uint32_t &imageIndex);
    VkRenderPass& getRenderPass() { return mRenderPass; }
    VkFramebuffer& getFramebuffer(int index) { return mSwapChainFramebuffers[index]; }
    VkExtent2D& getExtent() { return mSwapChainExtent; }

public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

private:
    void init();
    void createSwapChain();
    void createImageViews();
    // void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities);

private:
    Device& mDevice;
    VkExtent2D mWindowExtent;
    std::shared_ptr<SwapChain> oldSwapChain{nullptr};
    VkSwapchainKHR mSwapChain{};
    std::vector<VkImage> mSwapChainImages{};
    VkExtent2D mSwapChainExtent{};
    VkFormat mSwapChainFormat{};
    std::vector<VkImageView> mSwapChainImageViews{};
    VkRenderPass mRenderPass{};
    std::vector<VkFramebuffer> mSwapChainFramebuffers{};
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;
    std::vector<VkFence> mImagesInFlight;

    size_t currentFrame = 0;
};

} // namespace gfx

} // namespace horizon


#endif