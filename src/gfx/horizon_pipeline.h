#ifndef GFX_CORE_HORIZON_PIPELINE_H
#define GFX_CORE_HORIZON_PIPELINE_H

#include "gfx/horizon_device.h"

namespace horizon {

namespace gfx {

struct PipelineConfigInfo {
    PipelineConfigInfo() = default;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline {
public:
    Pipeline(Device &deviceRef, std::string vertFilePath, std::string fragFilePath, PipelineConfigInfo &pipelineConfig);
    ~Pipeline();

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, 
                                          std::vector<VkVertexInputBindingDescription>& vertexBindingDescriptions, 
                                          std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions);
    void bind(VkCommandBuffer commandBuffer);

private:
    void createGraphicsPipeline(std::string& vertFilePath, std::string& fragFilePath, PipelineConfigInfo& pipelineConfig);
    void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);

private:
    Device& mDevice;
    VkPipeline mGraphicsPipeline{};
};

} // namespace gfx

} // namespace horizon

#endif