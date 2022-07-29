#include "gfx/core/horizon_renderer.h"

namespace horizon {

namespace gfx {

Renderer::Renderer(Window &window, Device &device) : mWindow(window), mDevice(device) {
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer() {
    vkFreeCommandBuffers(mDevice.getDevice(), mDevice.getCommandPool(), static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
}

void Renderer::recreateSwapChain() {
    VkExtent2D extent = mWindow.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = mWindow.getExtent();
    }
    vkDeviceWaitIdle(mDevice.getDevice());
    if (mSwapChain == nullptr) {
        mSwapChain = std::make_unique<SwapChain>(mDevice, extent);
    } else {
        std::shared_ptr<SwapChain> oldSwapchain = std::move(mSwapChain);
        mSwapChain = std::make_unique<SwapChain>(extent, oldSwapchain);
        
        if (*mSwapChain.get() != *oldSwapchain.get()) {
            throw std::runtime_error("SwapChain image format has changed!");
        }
    }
}

void Renderer::createCommandBuffers() {
    mCommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

    if (vkAllocateCommandBuffers(mDevice.getDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

VkCommandBuffer Renderer::beginFrame() {
    ASSERT(!isFrameStarted, "Cant call begin frame while already in progress!");

    VkResult result = mSwapChain->acquireNextImage(currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }

    isFrameStarted = true;

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void Renderer::endFrame() {
    ASSERT(isFrameStarted, "Cant call endFrame if frame not in progress!");
    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }

    VkResult result = mSwapChain->submitCommandBuffer(commandBuffer, currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) { // mWindow.wasResized()) {}
        // mWindow.resetResizeFlag();
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    ASSERT(isFrameStarted, "Cant call beginSwapChainRenderPass if frame is not in progress!");
    ASSERT(commandBuffer == getCurrentCommandBuffer(), "Cant being render pass on a command buffer from a different frame!");
    
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = mSwapChain->getRenderPass();
    renderPassBeginInfo.framebuffer = mSwapChain->getFramebuffer(currentImageIndex);

    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = mSwapChain->getExtent();

    VkClearValue clearValues[2];
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(2);
    renderPassBeginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapChain->getExtent().width);
    viewport.height = static_cast<float>(mSwapChain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, mSwapChain->getExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor); 
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    ASSERT(isFrameStarted, "Cant call endSwapChainRenderPass if frame is not in progress!");
    ASSERT(commandBuffer == getCurrentCommandBuffer(), "Cant end renderpass on command buffer from a different frame!");
    vkCmdEndRenderPass(commandBuffer);
}

} // namespace gfx

} // namespace horizon

