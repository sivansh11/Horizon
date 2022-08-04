#ifndef RENDER_SYSTEMS_HORIZON_TEST_TEXTURE_RENDERER_H
#define RENDER_SYSTEMS_HORIZON_TEST_TEXTURE_RENDERER_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_renderer.h"
#include "gfx/horizon_pipeline.h"

#include "core/horizon_model.h"
#include "core/horizon_components.h"
#include "core/ecs.h"

namespace horizon {

struct UBO {
    glm::mat4 PV;
};

struct Push {
    glm::mat4 model;
};

class TestTextureRenderer {
public:
    TestTextureRenderer(gfx::Device& deviceRef, gfx::Renderer& rendererRef, VkDescriptorSetLayout descriptorSetLayout) : mDevice(deviceRef), mRenderer(rendererRef) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.offset = 0;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        pushConstantRange.size = sizeof(Push);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipeLineLayoutInfo{};
        pipeLineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeLineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipeLineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipeLineLayoutInfo.pushConstantRangeCount = 1;
        pipeLineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(mDevice.getDevice(), &pipeLineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        gfx::PipelineConfigInfo pipelineConfigInfo{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
        pipelineConfigInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
        pipelineConfigInfo.bindingDescriptions = Vertex::getBindingDescriptions();
        pipelineConfigInfo.renderPass = mRenderer.getSwapChainRenderPass();
        pipelineConfigInfo.pipelineLayout = mPipelineLayout;

        mPipeline = new gfx::Pipeline(mDevice,
                                     "shaders/example_test_texture.vert.spv",
                                     "shaders/example_test_texture.frag.spv",
                                     pipelineConfigInfo);
    }
    ~TestTextureRenderer() {
        vkDestroyPipelineLayout(mDevice.getDevice(), mPipelineLayout, nullptr);
        delete mPipeline;
    }

    void render(VkCommandBuffer commandBuffer, VkDescriptorSet set, ecs::Scene& scene) {
        mPipeline->bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &set, 0, nullptr);
        
        Push push{};
        for (auto [ent, model, transform] : ecs::SceneView<Model, Transform>(scene)) {
            push.model = transform.mat4();
            vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(push), &push);
            model.draw(commandBuffer);
        }        
    }

private:
    gfx::Device& mDevice;
    gfx::Renderer& mRenderer;

    VkPipelineLayout mPipelineLayout{};
    gfx::Pipeline *mPipeline;
};

} // namespace horizon

#endif