#ifndef GFX_RENDER_SYSTEMS_HORIZON_TEST_TRIANGLE_H
#define GFX_RENDER_SYSTEMS_HORIZON_TEST_TRIANGLE_H

#include "gfx/core/horizon_device.h"
#include "gfx/core/horizon_pipeline.h"

#include <memory>

namespace horizon {

namespace gfx {

class TestTriangle {
public:
    TestTriangle(Device &deviceRef, VkRenderPass renderPass) : mDevice(deviceRef) {
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

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig, vertexInputBindingDescription, vertexInputAttributeDescriptions);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = mPipelineLayout;
        mPipeline = std::make_unique<Pipeline>(mDevice, "../shaders/example_triangle.vert.spv", "../shaders/example_triangle.frag.spv", pipelineConfig);

    }
    ~TestTriangle() {
        mPipeline = nullptr;
        vkDestroyPipelineLayout(mDevice.getDevice(), mPipelineLayout, nullptr);
    }
    
    void render(VkCommandBuffer commandBuffer) {
        mPipeline->bind(commandBuffer);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

private:
    Device& mDevice;
    std::unique_ptr<Pipeline> mPipeline{};
    VkPipelineLayout mPipelineLayout{};
};

} // namespace gfx

} // namespace horizon

#endif