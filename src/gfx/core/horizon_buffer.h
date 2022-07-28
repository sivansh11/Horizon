#ifndef GFX_CORE_HORIZON_BUFFER_H
#define GFX_CORE_HORIZON_BUFFER_H

#include "gfx/core/horizon_device.h"

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

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unMap();

    

private:

};

} // namespace gfx

} // namespace horizon

#endif