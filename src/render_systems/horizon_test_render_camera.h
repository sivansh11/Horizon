#ifndef RENDER_SYSTEMS_HORIZON_TEST_RENDER_CAMERA_H
#define RENDER_SYSTEMS_HORIZON_TEST_RENDER_CAMERA_H

#include "gfx/horizon_device.h"
#include "gfx/horizon_pipeline.h"
#include "core/horizon_mesh.h"
#include "core/ecs.h"
#include "core/horizon_camera.h"
#include "core/horizon_transform.h"

namespace horizon {

struct Push {
    glm::mat4 PVM{};
};

class TestRenderCamera {
public:
    TestRenderCamera(gfx::Device& deviceRef, VkRenderPass renderPass) : mDevice(deviceRef) {
        VkPipelineLayoutCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(Push);
        pipelineCreateInfo.pushConstantRangeCount = 1;
        pipelineCreateInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(mDevice.getDevice(), &pipelineCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
        auto bindingDescription = horizon::Vertex::getBindingDescriptions();
        auto attributeDescription = horizon::Vertex::getAttributeDescriptions();

        gfx::PipelineConfigInfo pipelineConfig{};
        gfx::Pipeline::defaultPipelineConfigInfo(pipelineConfig, bindingDescription, attributeDescription);
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
    void render(VkCommandBuffer commandBuffer, ecs::Scene& scene, ecs::EntityID cameraEnt) {
        mPipeline->bind(commandBuffer);
        auto camera = scene.get<horizon::Camera>(cameraEnt);
        auto proj = camera.getProjection();
        auto view = camera.getView();
        Push push{};
        for (auto [ent, transform, mesh] : ecs::SceneView<Transform, horizon::Mesh>(scene)) {
            push.PVM = proj * view * transform.mat4();
            vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(Push), &push);
            mesh.bind(commandBuffer);
            mesh.draw(commandBuffer);
        }
    }
private:
    gfx::Device& mDevice;

    gfx::Pipeline *mPipeline{};
    VkPipelineLayout mPipelineLayout{};

};

} // namespace horizon


#endif