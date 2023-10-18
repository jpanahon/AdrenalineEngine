/*
    pipeline.cpp
    Adrenaline Engine

    This defines everything related to the graphics pipeline.
*/
#include "pipeline.h"
#include "info.h"

std::vector<uint32_t> Adren::Pipeline::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Failed to open file!";
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read((char *)buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule Adren::Pipeline::createShaderModule(const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;

    Adren::Tools::vibeCheck("SHADER MODULE", vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    return shaderModule;
}

void Adren::Pipeline::create(Swapchain& swapchain, VkDescriptorSetLayout& dLayout, VkRenderPass& renderpass) {
    auto vertShaderCode = readFile("../engine/resources/shaders/vert.spv");
    auto fragShaderCode = readFile("../engine/resources/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = Adren::Info::vertShaderStageInfo();
    vertShaderStageInfo.module = vertShaderModule;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = Adren::Info::fragShaderStageInfo();
    fragShaderStageInfo.module = fragShaderModule;

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = Adren::Info::inputAssembly();

    VkRect2D scissor = Adren::Info::scissor();
    scissor.extent = swapchain.extent;

    VkDynamicState states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineViewportStateCreateInfo viewportState = Adren::Info::viewportState();
    viewportState.pViewports = nullptr;
    viewportState.pScissors = nullptr;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = states;

    VkPipelineRasterizationStateCreateInfo rasterizer = Adren::Info::rasterizer();

    VkPipelineMultisampleStateCreateInfo multisampling = Adren::Info::multisampling();

    VkPipelineDepthStencilStateCreateInfo depthStencil = Adren::Info::depthStencil();

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Adren::Info::colorBlendAttachment();

    VkPipelineColorBlendStateCreateInfo colorBlending = Adren::Info::colorBlending();
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &dLayout;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);

    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    Tools::vibeCheck("PIPELINE LAYOUT", vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout));

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderpass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    Tools::vibeCheck("PIPELINE", vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle));

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Adren::Pipeline::cleanup() {
    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, handle, nullptr);
}
