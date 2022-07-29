#ifndef RENDER_SYSTEMS_HORIZON_TEST_TRIANGLE_H
#define RENDER_SYSTEMS_HORIZON_TEST_TRIANGLE_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_pipeline.h"
#include "gfx/horizon_buffer.h"

#include <memory>
#include <glm/glm.hpp>

namespace horizon {

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

class TestTriangle {
public:
    TestTriangle(gfx::Device &deviceRef, VkRenderPass renderPass) 
                : mDevice(deviceRef), 
                  vertexBuffer(mDevice, 
                               sizeof(Vertex) * vertices.size(), 
                               1,
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pSetLayouts = nullptr;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
        
        if (vkCreatePipelineLayout(mDevice.getDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions{};
        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription{};

        vertexInputBindingDescription.push_back({0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX});

        vertexInputAttributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos)});
        vertexInputAttributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});

        gfx::PipelineConfigInfo pipelineConfig{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfig, vertexInputBindingDescription, vertexInputAttributeDescriptions);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = mPipelineLayout;
        mPipeline = std::make_unique<gfx::Pipeline>(mDevice, "shaders/example_triangle.vert.spv", "shaders/example_triangle.frag.spv", pipelineConfig);

        vertexBuffer.map();
        vertexBuffer.writeToMappedBuffer((void*)vertices.data());
        
    }
    ~TestTriangle() {
        mPipeline = nullptr;
        vkDestroyPipelineLayout(mDevice.getDevice(), mPipelineLayout, nullptr);
    }
    
    void render(VkCommandBuffer commandBuffer) {
        mPipeline->bind(commandBuffer);

        VkBuffer buffers[] = {vertexBuffer.getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

private:
    gfx::Device& mDevice;
    std::unique_ptr<gfx::Pipeline> mPipeline{};
    VkPipelineLayout mPipelineLayout{};

    gfx::Buffer vertexBuffer;
};

} // namespace horizon

#endif