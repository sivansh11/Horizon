#ifndef GFX_HORIZON_BUFFER_H
#define GFX_HORIZON_BUFFER_H

#include "gfx/horizon_device.h"

namespace horizon {

namespace gfx {

class Buffer {
public:
    Buffer(Device& deviceRef, 
           VkDeviceSize instanceSize, 
           uint32_t instanceCount, 
           VkBufferUsageFlags usageFlags, 
           VkMemoryPropertyFlags memoryPropertyFlags, 
           VkDeviceSize minOffsetAlignment = 1);
    ~Buffer();

    void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void writeToMappedBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize = 0);
    void writeFromMappedBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize = 0);
    void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);    

    VkBuffer& getBuffer() { return mBuffer; }
    VkDeviceSize getCount() { return mInstanceCount; }

    static void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);

private:    
    static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
    void createBuffer(VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties);    

private:
    Device& mDevice;
    
    void* mMapped{nullptr};
    VkBuffer mBuffer{};
    VkDeviceMemory mMemory{};
    
    VkDeviceSize mBufferSize{};
    uint32_t mInstanceCount{};
    VkDeviceSize mInstanceSize{};
    VkDeviceSize mAlignmentSize{};
    VkBufferUsageFlags mUsageFlags{};
    VkMemoryPropertyFlags mMemoryPropertyFlags{};
};

} // namespace gfx

} // namespace horizon

#endif