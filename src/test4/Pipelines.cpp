#include "Pipelines.h"

#include <array>
#include <fstream>
#include <iostream>
#include <vector>

#include <volk.h>

#include "Init.h"
#include "Util.h"

namespace vkutil
{
VkShaderModule loadShaderModule(const char* filePath, VkDevice device)
{
    std::string dir = getExePath();
    std::string fullPathString = dir + "/" + filePath;
    std::ifstream file(fullPathString.c_str(), std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open " << fullPathString.c_str() << std::endl;
        std::exit(1);
    }

    const auto fileSize = file.tellg();
    std::vector<std::uint32_t> buffer(fileSize / sizeof(std::uint32_t));

    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();

    auto info = VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = buffer.size() * sizeof(std::uint32_t),
        .pCode = buffer.data(),
        //.flags = VK_SHADER_MODULE_CREATE_DEBUG_BIT_EXT; //避免优化 (不好使)
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &info, nullptr, &shaderModule) != VK_SUCCESS) {
        std::cout << "Failed to load " << filePath << std::endl;
        std::exit(1);
    }
    return shaderModule;
}

VkPipelineLayout createPipelineLayout(
    VkDevice device,
    std::span<const VkDescriptorSetLayout> layouts,
    std::span<const VkPushConstantRange> pushContantRanges)
{
    const auto createInfo = VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = (std::uint32_t)layouts.size(),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = (std::uint32_t)pushContantRanges.size(),
        .pPushConstantRanges = pushContantRanges.data(),
    };

    VkPipelineLayout layout;
    VK_CHECK(vkCreatePipelineLayout(device, &createInfo, nullptr, &layout));
    return layout;
}

} // end of namespace vkutil

PipelineBuilder::PipelineBuilder(VkPipelineLayout pipelineLayout) : pipelineLayout(pipelineLayout)
{
    clear();
}

void PipelineBuilder::clear()
{
    shaderStages.clear();

    inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    colorBlendAttachment = {};
    multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
}

VkPipeline PipelineBuilder::build(VkDevice device)
{
    const auto viewportState = VkPipelineViewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    const auto colorBlending = VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    const auto vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    auto dynamicStates = std::vector{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    if (dynamicDepth) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    }

    const auto dynamicInfo = VkPipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (std::uint32_t)dynamicStates.size(),
        .pDynamicStates = dynamicStates.data(),
    };

    const auto pipelineInfo = VkGraphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo,
        .stageCount = (std::uint32_t)shaderStages.size(),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicInfo,
        .layout = pipelineLayout,
    };

    VkPipeline pipeline;
    const auto res =
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    if (res != VK_SUCCESS) {
        std::cout << "Failed to create pipeline\n";
        return VK_NULL_HANDLE;
    }

    return pipeline;
}

PipelineBuilder& PipelineBuilder::setShaders(
    VkShaderModule vertexShader,
    VkShaderModule fragmentShader)
{
    shaderStages.clear();

    if (vertexShader) {
        shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
    }
    if (fragmentShader) {
        shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
    }

    return *this;
}

PipelineBuilder& PipelineBuilder::setShaders(
    VkShaderModule vertexShader,
    VkShaderModule geometryShader,
    VkShaderModule fragmentShader)
{
    shaderStages.clear();

    if (vertexShader) {
        shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));
    }
    if (geometryShader) {
        shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_GEOMETRY_BIT, geometryShader));
    }
    if (fragmentShader) {
        shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
    }

    return *this;
}

PipelineBuilder& PipelineBuilder::setInputTopology(VkPrimitiveTopology topology)
{
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(VkPolygonMode mode)
{
    rasterizer.polygonMode = mode;
    rasterizer.lineWidth = 1.f;

    return *this;
}

PipelineBuilder& PipelineBuilder::setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
{
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;

    return *this;
}

PipelineBuilder& PipelineBuilder::enableCulling()
{
    return setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

PipelineBuilder& PipelineBuilder::disableCulling()
{
    return setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
}

PipelineBuilder& PipelineBuilder::setMultisamplingNone()
{
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    return *this;
}

PipelineBuilder& PipelineBuilder::setMultisampling(VkSampleCountFlagBits samples)
{
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = samples;
    multisampling.minSampleShading = 1.0f;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    return *this;
}

PipelineBuilder& PipelineBuilder::disableBlending()
{
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    return *this;
}

PipelineBuilder& PipelineBuilder::enableBlending(
    VkBlendOp blendOp,
    VkBlendFactor srcBlendFactor,
    VkBlendFactor dstBlendFactor,
    VkBlendFactor srcAlphaBlendFactor,
    VkBlendFactor dstAlphaBlendFactor)
{
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.srcColorBlendFactor = srcBlendFactor;
    colorBlendAttachment.dstColorBlendFactor = dstBlendFactor;
    colorBlendAttachment.colorBlendOp = blendOp;
    colorBlendAttachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
    colorBlendAttachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
    colorBlendAttachment.alphaBlendOp = blendOp;

    return *this;
}

PipelineBuilder& PipelineBuilder::setColorAttachmentFormat(VkFormat format)
{
    colorAttachmentformat = format;

    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &colorAttachmentformat;

    return *this;
}

PipelineBuilder& PipelineBuilder::enableDepthTest(bool depthWriteEnable, VkCompareOp op)
{
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = depthWriteEnable;
    depthStencil.depthCompareOp = op;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;

    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthFormat(VkFormat format)
{
    renderInfo.depthAttachmentFormat = format;

    return *this;
}

PipelineBuilder& PipelineBuilder::enableDepthClamp()
{
    rasterizer.depthClampEnable = true;

    return *this;
}

PipelineBuilder& PipelineBuilder::disableDepthTest()
{
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;

    return *this;
}

PipelineBuilder& PipelineBuilder::enableDynamicDepth()
{
    dynamicDepth = true;

    return *this;
}

PipelineBuilder& PipelineBuilder::enableDepthBias(float constantFactor, float slopeFactor)
{
    rasterizer.depthBiasEnable = true;
    rasterizer.depthBiasConstantFactor = constantFactor;
    rasterizer.depthBiasSlopeFactor = slopeFactor;

    return* this;
}

ComputePipelineBuilder::ComputePipelineBuilder(VkPipelineLayout pipelineLayout) :
    pipelineLayout(pipelineLayout)
{}

ComputePipelineBuilder& ComputePipelineBuilder::setShader(VkShaderModule shaderModule)
{
    this->shaderModule = shaderModule;
    return *this;
}

VkPipeline ComputePipelineBuilder::build(VkDevice device)
{
    const auto pipelineCreateInfo = VkComputePipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage =
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                .module = shaderModule,
                .pName = "main",
            },
        .layout = pipelineLayout,
    };

    VkPipeline pipeline;
    VK_CHECK(
        vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, 0, &pipeline));
    return pipeline;
}
