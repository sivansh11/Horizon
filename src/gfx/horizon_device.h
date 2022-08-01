#ifndef GFX_HORIZON_DEVICE_H
#define GFX_HORIZON_DEVICE_H

#include "core/horizon_window.h"

#include <vector>

namespace horizon {

namespace gfx {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class Device {
public:
    Device(Window &window);
    ~Device();

    Device(const Device&) = delete;
    void operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    void operator=(const Device&&) = delete;

    inline VkDevice& getDevice() { return mDevice; }
    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(mPhysicalDevice); }
    VkSurfaceKHR& getSurface() { return mSurface; }
    QueueFamilyIndices getPhysicalDeviceQueueFamilies() { return findQueueFamilies(mPhysicalDevice); }
    VkCommandPool& getCommandPool() { return mCommandPool; }
    VkQueue& getGraphicsQueue() { return mGraphicsQueue; }
    VkQueue& getPresentQueue() { return mPresentQueue; }
    void waitIdle() { vkDeviceWaitIdle(mDevice); }
    VkFormat findSupportFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkCommandBuffer getSingleUseCommandBuffer();
    void endSingleUseCommandBuffer(VkCommandBuffer commandBuffer);

public:
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool checkValidationLayerSupport();
    std::vector<const char *> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    bool isPhysicalDeviceSuitable(VkPhysicalDevice &physicalDevice);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice &device);
    bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice &physicalDevice);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice &physicalDevice);

private:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    Window &mWindow;
    VkInstance mInstance{};
    VkDebugUtilsMessengerEXT mDebugMessenger{};
    VkSurfaceKHR mSurface{};
    VkPhysicalDevice mPhysicalDevice{};
    VkDevice mDevice{};
    VkCommandPool mCommandPool{};
    VkQueue mGraphicsQueue{};
    VkQueue mPresentQueue{};
};

} // namespace gfx

} // namespace horizon

#endif