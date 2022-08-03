#ifndef RENDER_SYSTEMS_POINT_LIGHT_RENDERER_H
#define RENDER_SYSTEMS_POINT_LIGHT_RENDERER_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_pipeline.h"
#include "gfx/horizon_renderer.h"

#include "core/horizon_components.h"

namespace horizon {

struct PointLightPushConstant {
    glm::mat4 view;
    PointLight pointLight;
};

class PointLightRenderer {
public:
    PointLightRenderer(gfx::Device& deviceRef, gfx::Renderer& rendererRef, VkDescriptorSetLayout descriptorSetLayout) 
        : mDevice(deviceRef),
          mRenderer(rendererRef) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstant);

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

        bindingDescription.clear();
        attributeDescription.clear();

        gfx::PipelineConfigInfo pipelineConfig{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions = attributeDescription;
        pipelineConfig.bindingDescriptions = bindingDescription;
        pipelineConfig.renderPass = mRenderer.getSwapChainRenderPass();
        pipelineConfig.pipelineLayout = mPipelineLayout;

        mPipeline = std::make_unique<gfx::Pipeline>(mDevice, 
                                                    "shaders/point_light.vert.spv",
                                                    "shaders/point_light.frag.spv",
                                                    pipelineConfig);
    }

    ~PointLightRenderer() {
        vkDestroyPipelineLayout(mDevice.getDevice(), mPipelineLayout, nullptr);
    }

    void render(VkCommandBuffer commandBuffer, VkDescriptorSet set, ecs::Scene& scene) {
        // bind pipeline
        // bind descriptorset for uniforms
        // push constants

        mPipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &set, 0, nullptr);
        
        ecs::EntityID cameraEnt;
        for (auto ent : ecs::SceneView<>(scene)) if (scene.has<Camera>(ent)) {
            cameraEnt = ent;
        }
        auto& camera = scene.get<Camera>(cameraEnt);

        PointLightPushConstant pushConstant{};
        pushConstant.view = camera.getView();
        for (auto [ent, pointLight] : ecs::SceneView<PointLight>(scene)) {
            pushConstant.pointLight = pointLight;
            vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PointLightPushConstant), &pushConstant);
            vkCmdDraw(commandBuffer, 6, 1, 0, 0);
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