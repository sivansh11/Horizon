#ifndef RENDER_SYSTEMS_HORIZON_TEST_TRIANGLES_2_H
#define RENDER_SYSTEMS_HORIZON_TEST_TRIANGLES_2_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_pipeline.h"
#include "horizon_mesh.h"

namespace horizon {

std::vector<Vertex> vertices {
    {{-.5, .5, 0}, {1.0, 0, 0}, {0, 0, 0}, {0, 0}},
    {{.5, .5, 0}, {0, 1.0, 0}, {0, 0, 0}, {0, 0}},
    {{0, -.5, 0}, {0, 0, 1.0}, {0, 0, 0}, {0, 0}}
};

std::vector<uint32_t> indices {
    0, 1, 2
};

class TestTriangle2 {
public:
    TestTriangle2(gfx::Device& deviceRef, VkRenderPass renderPass) : mDevice(deviceRef) {
        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = nullptr;
        createInfo.setLayoutCount = 0;
        createInfo.pSetLayouts = nullptr;
        if (vkCreatePipelineLayout(mDevice.getDevice(), &createInfo, nullptr, &pipelineLayout) != VK_SUCCESS ) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions = Vertex::getAttributeDescriptions();
        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescription = Vertex::getBindingDescriptions();
        
        gfx::PipelineConfigInfo pipelineConfig{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfig, vertexInputBindingDescription, vertexInputAttributeDescriptions);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        
        pipeline = new gfx::Pipeline(mDevice,
                                     "shaders/example_triangle_2.vert.spv",
                                     "shaders/example_triangle_2.frag.spv", 
                                     pipelineConfig);

        mesh = new Mesh(mDevice,
                        &vertices,
                        &indices);
    }
    ~TestTriangle2() {
        delete mesh;
        delete pipeline;
        vkDestroyPipelineLayout(mDevice.getDevice(), pipelineLayout, nullptr);
    }

    void render(VkCommandBuffer commandBuffer) {
        pipeline->bind(commandBuffer);
        mesh->bind(commandBuffer);
        mesh->draw(commandBuffer);
    }

private:
    gfx::Device& mDevice;
    
    Mesh *mesh;
    gfx::Pipeline *pipeline;
    VkPipelineLayout pipelineLayout{};
};

} // namespace horizon


#endif