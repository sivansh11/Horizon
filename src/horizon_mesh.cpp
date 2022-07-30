#include "horizon_mesh.h"

#include "debug.h"

namespace horizon {

Mesh::Mesh(gfx::Device& deviceRef, std::vector<Vertex> *vertices, std::vector<uint32_t> *indices) : mDevice(deviceRef) {
    if (vertices != nullptr) 
        createVertexBuffer(vertices);
    if (indices != nullptr) 
        createIndexBuffer(indices);
    
}

Mesh::~Mesh() {

}

void Mesh::createVertexBuffer(std::vector<Vertex> *vertices) {
    ASSERT(vertices->size() >= 3, "Vertex count must be at least 3!");
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();
    uint32_t vertexSize = sizeof(Vertex);

    gfx::Buffer stagingBuffer(mDevice, 
                              vertexSize, 
                              vertices->size(), 
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.writeToMappedBuffer((void*)vertices->data());

    vertexBuffer = std::make_unique<gfx::Buffer>(mDevice,
                                                 vertexSize,
                                                 vertices->size(),
                                                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    gfx::Buffer::copyBuffer(stagingBuffer, *vertexBuffer, bufferSize);
}

void Mesh::createIndexBuffer(std::vector<uint32_t> *indices) {
    VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();
    uint32_t indexSize = sizeof(uint32_t);

    gfx::Buffer stagingBuffer(mDevice,
                              indexSize,
                              indices->size(),
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.map();
    stagingBuffer.writeToMappedBuffer((void*)indices->data());

    indexBuffer = std::make_unique<gfx::Buffer>(mDevice,
                                                indexSize,
                                                indices->size(),
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    gfx::Buffer::copyBuffer(stagingBuffer, *indexBuffer, bufferSize);
}

void Mesh::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (indexBuffer != nullptr && indexBuffer->getCount() > 0) {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void Mesh::draw(VkCommandBuffer commandBuffer) {
    if (indexBuffer != nullptr && indexBuffer->getCount() > 0) {
        vkCmdDrawIndexed(commandBuffer, indexBuffer->getCount(), 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, vertexBuffer->getCount(), 1, 0, 0);
    }
}

std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
    return attributeDescriptions;
}

} // namespace horizon
