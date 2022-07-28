#include "gfx/core/horizon_swapchain.h"

#include <iostream>
#include <limits>
#include <algorithm>

namespace horizon {

namespace gfx {

SwapChain::SwapChain(Device &deviceRef, VkExtent2D windowExtent) : mDevice(deviceRef), mWindowExtent(windowExtent) {
    init();
}

SwapChain::SwapChain(VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous) : mDevice(previous->mDevice), mWindowExtent(windowExtent) {
    oldSwapChain = previous;
    init();
    oldSwapChain = nullptr;
}

SwapChain::~SwapChain() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(mDevice.getDevice(), mImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mDevice.getDevice(), mRenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(mDevice.getDevice(), mInFlightFences[i], nullptr);
    }
    for (auto framebuffer : mSwapChainFramebuffers) {
        vkDestroyFramebuffer(mDevice.getDevice(), framebuffer, nullptr);
    }
    vkDestroyRenderPass(mDevice.getDevice(), mRenderPass, nullptr);
    for (auto imageView : mSwapChainImageViews) {
        vkDestroyImageView(mDevice.getDevice(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(mDevice.getDevice(), mSwapChain, nullptr);
}

void SwapChain::init() {
    createSwapChain();
    createImageViews();
    // createDepthResources();
    createRenderPass();
    createFramebuffers();
    createSyncObjects();
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::cout << "Present mode: Mailbox\n"; 
        }
    }

    std::cout << "Present mode: V-Sync\n"; 
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = mWindowExtent;
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

void SwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = mDevice.getSwapChainSupport();
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.minImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = mDevice.getSurface();
    
    swapChainCreateInfo.minImageCount = imageCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = mDevice.getPhysicalDeviceQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};
    if (indices.graphicsFamily != indices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = VK_TRUE;

    swapChainCreateInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->mSwapChain;

    if (vkCreateSwapchainKHR(mDevice.getDevice(), &swapChainCreateInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain!");
    }
    vkGetSwapchainImagesKHR(mDevice.getDevice(), mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice.getDevice(), mSwapChain, &imageCount, mSwapChainImages.data());

    mSwapChainFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
}

void SwapChain::createImageViews() {
    mSwapChainImageViews.resize(mSwapChainImages.size());
    for (size_t i = 0; i < mSwapChainImages.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = mSwapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = mSwapChainFormat;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(mDevice.getDevice(), &imageViewCreateInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view!");
        }
    }
}

void SwapChain::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapChainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(mDevice.getDevice(), &renderPassCreateInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void SwapChain::createFramebuffers() {
    mSwapChainFramebuffers.resize(mSwapChainImageViews.size());
    for (size_t i = 0; i < mSwapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            mSwapChainImageViews[i]
        };
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = mRenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = mSwapChainExtent.width;
        framebufferCreateInfo.height = mSwapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(mDevice.getDevice(), &framebufferCreateInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffers!");
        }
    }
}

void SwapChain::createSyncObjects() {
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    mImagesInFlight.resize(mSwapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(mDevice.getDevice(), &semaphoreCreateInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice.getDevice(), &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice.getDevice(), &fenceCreateInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

VkResult SwapChain::acquireNextImage(uint32_t &imageIndex) {
    vkWaitForFences(mDevice.getDevice(), 1, &mInFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    VkResult result = vkAcquireNextImageKHR(mDevice.getDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    return result;
}

VkResult SwapChain::submitCommandBuffer(VkCommandBuffer &buffer, uint32_t &imageIndex) {
    if (mImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(mDevice.getDevice(), 1, &mImagesInFlight[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }
    mImagesInFlight[imageIndex] = mInFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice.getDevice(), 1, &mInFlightFences[currentFrame]);
    if (vkQueueSubmit(mDevice.getGraphicsQueue(), 1, &submitInfo, mInFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {mSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(mDevice.getPresentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

} // namespace gfx

} // namespace horizon

