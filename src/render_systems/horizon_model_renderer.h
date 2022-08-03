#ifndef RENDER_SYSTEMS_MODEL_RENDERER_H
#define RENDER_SYSTEMS_MODEL_RENDERER_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_pipeline.h"
#include "gfx/horizon_renderer.h"

#include "core/horizon_components.h"

namespace horizon {

struct ModelPushConstant {
    glm::mat4 model{};
};

class ModelRenderer {
public:
    ModelRenderer(gfx::Device& deviceRef, gfx::Renderer& rendererRef, VkDescriptorSetLayout descriptorSetLayout) 
        : mDevice(deviceRef),
          mRenderer(rendererRef) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ModelPushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

        pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

        if (vkCreatePipelineLayout(mDevice.getDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        auto bindingDescription = Vertex::getBindingDescriptions();
        auto attributeDescription = Vertex::getAttributeDescriptions();

        gfx::PipelineConfigInfo pipelineConfig{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions = attributeDescription;
        pipelineConfig.bindingDescriptions = bindingDescription;
        pipelineConfig.renderPass = mRenderer.getSwapChainRenderPass();
        pipelineConfig.pipelineLayout = mPipelineLayout;

        mPipeline = std::make_unique<gfx::Pipeline>(mDevice, 
                                                    "shaders/model.vert.spv",
                                                    "shaders/model.frag.spv",
                                                    pipelineConfig);
    }

    ~ModelRenderer() {
        vkDestroyPipelineLayout(mDevice.getDevice(), mPipelineLayout, nullptr);
    }

    void render(VkCommandBuffer commandBuffer, VkDescriptorSet set, ecs::Scene& scene) {
        // bind pipeline
        // bind descriptorset for uniforms
        // push constants

        mPipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &set, 0, nullptr);

        ModelPushConstant pushConstant{};
        for (auto [ent, transform, model] : ecs::SceneView<Transform, Model>(scene)) {
            pushConstant.model = transform.mat4();
            vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(ModelPushConstant), &pushConstant);
            model.draw(commandBuffer);
        }

    }

private:
    gfx::Device& mDevice;
    gfx::Renderer& mRenderer;
    
    VkPipelineLayout mPipelineLayout;
    std::unique_ptr<gfx::Pipeline> mPipeline;
};

} // namespace horizon

#endif