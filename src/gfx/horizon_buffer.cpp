#include "gfx/horizon_buffer.h"

#include "debug.h"

#include <cstring>

namespace horizon {

namespace gfx {

Buffer::Buffer(Device& deviceRef,
               VkDeviceSize instanceSize,
               uint32_t instanceCount,
               VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memoryPropertyFlags,
               VkDeviceSize minOffsetAlignment)
               : mDevice(deviceRef),
                 mInstanceSize(instanceSize),
                 mInstanceCount(instanceCount),
                 mUsageFlags(usageFlags),
                 mMemoryPropertyFlags(memoryPropertyFlags) {
    mAlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
    mBufferSize = mAlignmentSize * instanceCount;
    createBuffer(mBufferSize, mUsageFlags, mMemoryPropertyFlags);
}

Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(mDevice.getDevice(), mBuffer, nullptr);
    vkFreeMemory(mDevice.getDevice(), mMemory, nullptr);
}

VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
    if (minOffsetAlignment > 0) {
        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
    } return instanceSize;
}

void Buffer::createBuffer(VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(mDevice.getDevice(), &bufferCreateInfo, nullptr, &mBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements memReqs{};
    vkGetBufferMemoryRequirements(mDevice.getDevice(), mBuffer, &memReqs);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = mDevice.findMemoryType(memReqs.memoryTypeBits, properties);

    if (vkAllocateMemory(mDevice.getDevice(), &memAllocInfo, nullptr, &mMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }
    vkBindBufferMemory(mDevice.getDevice(), mBuffer, mMemory, 0);
}

void Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    ASSERT(mBuffer && mMemory, "Cant call map before buffer creation!");
    if (vkMapMemory(mDevice.getDevice(), mMemory, offset, size, 0, &mMapped) != VK_SUCCESS) {
        throw std::runtime_error("Failed to map memory!");
    }
}

void Buffer::unmap() {
    if (mMapped) {
        vkUnmapMemory(mDevice.getDevice(), mMemory);
        mMapped = nullptr;
    }
}

void Buffer::writeToMappedBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
    ASSERT(mMapped, "Cant copy to unmapped memory!");
    if (size == VK_WHOLE_SIZE) {
        std::memcpy(mMapped, data, mBufferSize);
    } else {
        char *memOffset = (char*)mMapped;
        memOffset += offset;
        std::memcpy(memOffset, data, size);
    }
}

void Buffer::writeFromMappedBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
    ASSERT(mMapped, "Cant copy from unmmaped memory!");
    if (size != VK_WHOLE_SIZE) {
        throw std::runtime_error("Test this before using!");
    }
    if (size == VK_WHOLE_SIZE) {
        std::memcpy(data, mMapped, mBufferSize);
    } else {
        char *memOffset = (char*)data;
        memOffset += offset;
        std::memcpy(memOffset, mMapped, size);
    }
}

void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    ASSERT(mMapped, "Cant flush unmapped memory!");
    VkMappedMemoryRange mappedRange{};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = mMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    if (vkFlushMappedMemoryRanges(mDevice.getDevice(), 1, &mappedRange) != VK_SUCCESS) {
        throw std::runtime_error("Failed to flush mapped memory!");
    }
}

void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    ASSERT(mMapped, "Cant invalidate unmapped memory!");
    VkMappedMemoryRange mappedRange{};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = mMemory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    if (vkInvalidateMappedMemoryRanges(mDevice.getDevice(), 1, &mappedRange) != VK_SUCCESS) {
        throw std::runtime_error("Failed to invalidate mapped memory!");
    } 
}

void Buffer::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size) {
    Device& device = srcBuffer.mDevice;
    VkCommandBuffer commandBuffer = device.getSingleUseCommandBuffer();
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.mBuffer, dstBuffer.mBuffer, 1, &copyRegion);
    device.endSingleUseCommandBuffer(commandBuffer);
}

} // namespace gfx

} // namespace horizon
