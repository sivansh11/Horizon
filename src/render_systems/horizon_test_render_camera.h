#ifndef RENDER_SYSTEMS_HORIZON_TEST_RENDER_CAMERA_H
#define RENDER_SYSTEMS_HORIZON_TEST_RENDER_CAMERA_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_pipeline.h"
#include "core/horizon_model.h"
#include "core/ecs.h"
#include "core/horizon_camera.h"
#include "core/horizon_transform.h"

namespace horizon {

struct Push {
    glm::mat4 model{};
};

struct Ubo {
    glm::mat4 PV{};
};

class TestRenderCamera {
public:
    TestRenderCamera(gfx::Device& deviceRef, VkRenderPass renderPass, VkDescriptorSetLayout setLayout) : mDevice(deviceRef) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(Push);
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{setLayout};
        VkPipelineLayoutCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineCreateInfo.pushConstantRangeCount = 1;
        pipelineCreateInfo.pPushConstantRanges = &pushConstantRange;
        pipelineCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        if (vkCreatePipelineLayout(mDevice.getDevice(), &pipelineCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
        auto bindingDescription = horizon::Vertex::getBindingDescriptions();
        auto attributeDescription = horizon::Vertex::getAttributeDescriptions();

        gfx::PipelineConfigInfo pipelineConfig{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions = attributeDescription;
        pipelineConfig.bindingDescriptions = bindingDescription;
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = mPipelineLayout;

        mPipeline = new gfx::Pipeline(mDevice,
                                      "shaders/example_test_render_camera.vert.spv",
                                      "shaders/example_test_render_camera.frag.spv",
                                      pipelineConfig);
    }
    ~TestRenderCamera() {
        vkDestroyPipelineLayout(mDevice.getDevice(), mPipelineLayout, nullptr);
        delete mPipeline;
    }
    void render(VkCommandBuffer commandBuffer, VkDescriptorSet set, ecs::Scene& scene, ecs::EntityID cameraEnt) {
        mPipeline->bind(commandBuffer);
        auto camera = scene.get<horizon::Camera>(cameraEnt);
        
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &set, 0, nullptr);

        Push push;
        for (auto [ent, transform, model] : ecs::SceneView<Transform, horizon::Model>(scene)) {
            push.model = transform.mat4();
            vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(Push), &push);
            model.draw(commandBuffer);
        }
    }
private:
    gfx::Device& mDevice;

    gfx::Pipeline *mPipeline{};
    VkPipelineLayout mPipelineLayout{};

};

} // namespace horizon


#endif