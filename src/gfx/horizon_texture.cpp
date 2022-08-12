#include "gfx/horizon_texture.h"

#include "core/debug.h"

#include "gfx/horizon_buffer.h"

#include "stb_image.h"

#include <cmath>

namespace horizon {

namespace gfx
{

Texture2D::Texture2D(Device& deviceRef, std::string file) : mDevice(deviceRef) {
    int channels, bytesPerPixel;
    uint8_t *data = stbi_load(file.c_str(), &mWidth, &mHeight, &bytesPerPixel, 4);
    mMiplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(mWidth, mHeight)))) + 1;
    ASSERT(data, stbi_failure_reason());
    Buffer stagingBuffer(mDevice, 
                         4,
                         static_cast<uint32_t>(mWidth * mHeight),
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.writeToMappedBuffer(data);
    mImageFormat = VK_FORMAT_R8G8B8A8_SRGB;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = mImageFormat;
    imageInfo.mipLevels = mMiplevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.extent = {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight), 1};
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if (vkCreateImage(mDevice.getDevice(), &imageInfo, nullptr, &mImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }

    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements(mDevice.getDevice(), mImage, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = mDevice.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(mDevice.getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    if (vkBindImageMemory(mDevice.getDevice(), mImage, mImageMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Failed to bind image memory!");
    }

    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer, mImage, static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight), 1);

    generateMipMaps();

    mImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mMiplevels);
    samplerInfo.maxAnisotropy = 4.0;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    if (vkCreateSampler(mDevice.getDevice(), &samplerInfo, nullptr, &mSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create sampler!");
    }

    VkImageViewCreateInfo imageViewInfo {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = mImageFormat;
    imageViewInfo.components = {};
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.levelCount = mMiplevels;
    imageViewInfo.image = mImage;

    if (vkCreateImageView(mDevice.getDevice(), &imageViewInfo, nullptr, &mImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view!");
    }

    stbi_image_free(data);
}   

Texture2D::~Texture2D() {
    vkDestroyImage(mDevice.getDevice(), mImage, nullptr);
    vkFreeMemory(mDevice.getDevice(), mImageMemory, nullptr);
    vkDestroyImageView(mDevice.getDevice(), mImageView, nullptr);
    vkDestroySampler(mDevice.getDevice(), mSampler, nullptr);
}

void Texture2D::copyBufferToImage(Buffer& buffer, VkImage image, uint32_t mWidth, uint32_t mHeight, uint32_t layerCount) {
    auto commandBuffer = mDevice.getSingleUseCommandBuffer();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;
    
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {mWidth, mHeight, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer.getBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    mDevice.endSingleUseCommandBuffer(commandBuffer);
}

void Texture2D::transitionImageLayout(VkImageLayout oldImageLayout, VkImageLayout newImageLayout) {
    auto commandBuffer = mDevice.getSingleUseCommandBuffer();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldImageLayout;
    barrier.newLayout = newImageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = mImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mMiplevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::runtime_error("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    mDevice.endSingleUseCommandBuffer(commandBuffer);
}   

void Texture2D::generateMipMaps() {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(mDevice.getPhysicalDevice(), mImageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = mDevice.getSingleUseCommandBuffer();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = mImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = mWidth;
    int32_t mipHeight = mHeight;

    for (uint32_t i = 1; i < mMiplevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mMiplevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    mDevice.endSingleUseCommandBuffer(commandBuffer);
}

} // namespace gfx

} // namespace horizon

