#ifndef GFX_HORIZON_DESCRIPTOR_H
#define GFX_HORIZON_DESCRIPTOR_H

#include "gfx/horizon_device.h"

#include <unordered_map>

namespace horizon {

namespace gfx {

class DescriptorSetLayout {
public:
    using BindingMap = std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>;
    class Builder {
    public:
        Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
        BindingMap& getBindings() { return bindings; }
    private:
        BindingMap bindings;
    };
    DescriptorSetLayout(Device& deviceRef, VkDescriptorSetLayoutCreateFlags flags, BindingMap bindings);
    ~DescriptorSetLayout();

    VkDescriptorSetLayout getSetLayout() { return mDescriptorSetLayout; }

private:
    Device& mDevice;
    VkDescriptorSetLayout mDescriptorSetLayout{};
    BindingMap bindings;

    friend class DescriptorWriter;
};

class DescriptorPool {
public:
    class Builder {
    public:
        Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        std::vector<VkDescriptorPoolSize>& getPoolSizes() { return poolSizes; }
    private:
        std::vector<VkDescriptorPoolSize> poolSizes;
    };
    DescriptorPool(Device& deviceRef, VkDescriptorPoolCreateFlags poolFlags, std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets = 1000);
    ~DescriptorPool();

    bool allocateDescriptor(const VkDescriptorSetLayout DescriptorSetLayout, VkDescriptorSet &descriptor);
    void freeDescriptor(VkDescriptorSet *descriptors, uint32_t size);

private:
    Device& mDevice;
    VkDescriptorPool mDescriptorPool;

    friend class DescriptorWriter;
};

class DescriptorWriter {
public:
    DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

    DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);

    bool pushWrites(VkDescriptorSet &descriptor);

private:
    DescriptorSetLayout& mSetLayout;
    DescriptorPool& mPool;
    std::vector<VkWriteDescriptorSet> mWrites;
};

} // namespace gfx

} // namespace horizon


#endif