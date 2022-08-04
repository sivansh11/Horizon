#include "gfx/horizon_descriptor.h"

#include "core/debug.h"

namespace horizon {

namespace gfx {

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
    ASSERT(mBindings.find(binding) == mBindings.end(), "Binding already exist!");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    mBindings[binding] = layoutBinding;
    return *this;
}

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addFlags(VkDescriptorSetLayoutCreateFlags flags) {
    mFlags |= flags;
    return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() {
    return std::make_unique<DescriptorSetLayout>(mDevice, mFlags, mBindings);
}

DescriptorSetLayout::DescriptorSetLayout(Device& deviceRef, VkDescriptorSetLayoutCreateFlags flags, BindingMap bindings) : mDevice(deviceRef) {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
    descriptorSetLayoutInfo.flags = flags;
    if (vkCreateDescriptorSetLayout(mDevice.getDevice(), &descriptorSetLayoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
    DescriptorSetLayout::bindings = bindings;
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(mDevice.getDevice(), mDescriptorSetLayout, nullptr);
}

DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
    mPoolSizes.push_back({descriptorType, count});
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::addFlags(VkDescriptorPoolCreateFlags flags) {
    mFlags |= flags;
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t maxSets) {
    mMaxSets = maxSets;
    return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() {
    return std::make_unique<DescriptorPool>(mDevice, mFlags, mPoolSizes, mMaxSets);
}

DescriptorPool::DescriptorPool(Device& deviceRef, VkDescriptorPoolCreateFlags poolFlags, std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets) : mDevice(deviceRef) {
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(mDevice.getDevice(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(mDevice.getDevice(), mDescriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    if (vkAllocateDescriptorSets(mDevice.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void DescriptorPool::freeDescriptor(VkDescriptorSet *descriptors, uint32_t size) {
    vkFreeDescriptorSets(mDevice.getDevice(), mDescriptorPool, size, descriptors);
}

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool) : mSetLayout(setLayout), mPool(pool) {}

DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
    ASSERT(mSetLayout.bindings.find(binding) != mSetLayout.bindings.end(), "Binding specified does not exist in the set layout");
    auto &bindingDescription = mSetLayout.bindings[binding];

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    mWrites.push_back(write);
    return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo) {
    ASSERT(mSetLayout.bindings.find(binding) != mSetLayout.bindings.end(), "Binding specified does not exist in the set layout");
    auto &bindingDescription = mSetLayout.bindings[binding];
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    mWrites.push_back(write);
    return *this;
}

bool DescriptorWriter::pushWrites(VkDescriptorSet &descriptor) {
    if (!mPool.allocateDescriptor(mSetLayout.mDescriptorSetLayout, descriptor)) {
        return false;
    }
    for (auto& write : mWrites) {
        write.dstSet = descriptor;
    }
    vkUpdateDescriptorSets(mPool.mDevice.getDevice(), mWrites.size(), mWrites.data(), 0, nullptr);
    return true;
}

} // namespace gfx

} // namespace horizon
