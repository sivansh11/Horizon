#include "gfx/horizon_pipeline.h"

#include "core/debug.h"

#include "utils/horizon_helper.h"

#include <shaderc/shaderc.hpp>

namespace horizon {

namespace gfx {

Pipeline::Pipeline(Device &deviceRef, std::string vertFilePath, std::string fragFilePath, PipelineConfigInfo &pipelineConfig) : mDevice(deviceRef) {
    createGraphicsPipeline(vertFilePath, fragFilePath, pipelineConfig);    
}

Pipeline::~Pipeline() {
    vkDestroyPipeline(mDevice.getDevice(), mGraphicsPipeline, nullptr);
}

void Pipeline::createGraphicsPipeline(std::string& vertFilePath, std::string& fragFilePath, PipelineConfigInfo& pipelineConfig) {
    ASSERT(pipelineConfig.pipelineLayout != VK_NULL_HANDLE, "Cannot create graphics pipeline: no pipelineLayout provided in pipelineConfig!");
    ASSERT(pipelineConfig.renderPass != VK_NULL_HANDLE, "Cannot create graphics pipeline: no renderPass provided in the pipelineConfig!");

    // std::vector<char> vertCode = utils::readFile(vertFilePath.c_str(), utils::BINARY);
    // std::vector<char> fragCode = utils::readFile(fragFilePath.c_str(), utils::BINARY);
    auto vertCode = ShaderCompiler::getSpv(vertFilePath);
    auto fragCode = ShaderCompiler::getSpv(fragFilePath);


    VkShaderModule vertShaderModule{};
    VkShaderModule fragShaderModule{};

    createShaderModule(vertCode, vertShaderModule);
    createShaderModule(fragCode, fragShaderModule);
    
    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(pipelineConfig.attributeDescriptions.size());
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(pipelineConfig.bindingDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = pipelineConfig.attributeDescriptions.data();
    vertexInputInfo.pVertexBindingDescriptions = pipelineConfig.bindingDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &pipelineConfig.inputAssemblyInfo;
    pipelineCreateInfo.pViewportState = &pipelineConfig.viewportInfo;
    pipelineCreateInfo.pRasterizationState = &pipelineConfig.rasterizationInfo;
    pipelineCreateInfo.pMultisampleState = &pipelineConfig.multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &pipelineConfig.depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &pipelineConfig.colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &pipelineConfig.dynamicStateInfo;
    pipelineCreateInfo.layout = pipelineConfig.pipelineLayout;
    pipelineCreateInfo.renderPass = pipelineConfig.renderPass;
    pipelineCreateInfo.subpass = pipelineConfig.subpass;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(mDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(mDevice.getDevice(), vertShaderModule, nullptr);
    vkDestroyShaderModule(mDevice.getDevice(), fragShaderModule, nullptr);
}

void Pipeline::createShaderModule(const std::vector<uint32_t>& code, VkShaderModule& shaderModule) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = sizeof(uint32_t) * code.size();
    shaderModuleCreateInfo.pCode = code.data();

    if (vkCreateShaderModule(mDevice.getDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
}

void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;

    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
    configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

    configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                                     VK_COLOR_COMPONENT_G_BIT | 
                                                     VK_COLOR_COMPONENT_B_BIT |
                                                     VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {};  // Optional
    configInfo.depthStencilInfo.back = {};   // Optional

    configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.flags = 0;
}

void Pipeline::bind(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}

// Pipeline::ShaderCompiler Pipeline::ShaderCompiler::sShaderCompiler;

Pipeline::ShaderCompiler::~ShaderCompiler() {

}

std::vector<uint32_t> Pipeline::ShaderCompiler::getSpv(std::string& file) {
    std::string code = utils::readFile(file.c_str(), utils::TEXT);
    std::string fileName = file.substr(file.find_last_of('/') + 1);
    shaderc_shader_kind type;
    if (file.find(".vert") != std::string::npos) type = shaderc_vertex_shader;
    if (file.find(".frag") != std::string::npos) type = shaderc_fragment_shader;
    if (file.find(".comp") != std::string::npos) type = shaderc_compute_shader;
    if (file.find(".geom") != std::string::npos) type = shaderc_geometry_shader;

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto preprocess = compiler.PreprocessGlsl(code, type, fileName.c_str(), options);
    code = {preprocess.cbegin(), preprocess.cend()};

    auto module = compiler.CompileGlslToSpv(code, type, fileName.c_str(), options); 
    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error(module.GetErrorMessage());
    }
    return {module.cbegin(), module.cend()};
} 

} // namespace gfx

} // namespace horizon
