#ifndef GFX_HORIZON_DESCRIPTOR_H
#define GFX_HORIZON_DESCRIPTOR_H

#include "gfx/horizon_device.h"

#include <unordered_map>
#include <memory>

namespace horizon {

namespace gfx {

class DescriptorSetLayout {
public:
    using BindingMap = std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>;
    class Builder {
    public:
        Builder(Device& deviceRef) : mDevice(deviceRef) {}
        Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
        Builder& addFlags(VkDescriptorSetLayoutCreateFlags flags);
        std::unique_ptr<DescriptorSetLayout> build();
        BindingMap& getBindings() { return mBindings; }
    private:
        Device& mDevice;
        BindingMap mBindings;
        VkDescriptorSetLayoutCreateFlags mFlags{0};
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
        Builder(Device& deviceRef) : mDevice(deviceRef) {}
        Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder& addFlags(VkDescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t maxSets);
        std::unique_ptr<DescriptorPool> build();
        std::vector<VkDescriptorPoolSize>& getPoolSizes() { return mPoolSizes; }
    private:
        Device& mDevice;
        uint32_t mMaxSets{1000};
        VkDescriptorPoolCreateFlags mFlags{0};
        std::vector<VkDescriptorPoolSize> mPoolSizes;
    };
    DescriptorPool(Device& deviceRef, VkDescriptorPoolCreateFlags poolFlags, std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);
    ~DescriptorPool();

    VkDescriptorPool& getDescriptorPool() { return mDescriptorPool; }

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
    DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool pushWrites(VkDescriptorSet &descriptor);

private:
    DescriptorSetLayout& mSetLayout;
    DescriptorPool& mPool;
    std::vector<VkWriteDescriptorSet> mWrites;
};

} // namespace gfx

} // namespace horizon


#endif