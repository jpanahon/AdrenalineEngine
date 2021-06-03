/*
    pipeline.cpp
    Adrenaline Engine
 
    This has the functions needed to create the graphics pipeline.
    Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#include "pipeline.h"
#include "info.h"

void Adren::Pipeline::createGraphicsPipeline() {
    auto vertShaderCode = readFile("engine/resources/shaders/vert.spv");
    auto fragShaderCode = readFile("engine/resources/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, renderer.device);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, renderer.device);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = Adren::Info::vertShaderStageInfo();
    vertShaderStageInfo.module = vertShaderModule;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = Adren::Info::fragShaderStageInfo();
    fragShaderStageInfo.module = fragShaderModule;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = Adren::Info::inputAssembly();
    
    VkViewport viewport = Adren::Info::viewport();
    viewport.width = (float) renderer.swapChainExtent.width;
    viewport.height = (float) renderer.swapChainExtent.height;

    VkRect2D scissor = Adren::Info::scissor();
    scissor.extent = renderer.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = Adren::Info::viewportState();
    viewportState.pViewports = &viewport;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = Adren::Info::rasterizer();

    VkPipelineMultisampleStateCreateInfo multisampling = Adren::Info::multisampling();
    
    VkPipelineDepthStencilStateCreateInfo depthStencil = Adren::Info::depthStencil();

    VkPipelineColorBlendAttachmentState colorBlendAttachment = Adren::Info::colorBlendAttachment();

    VkPipelineColorBlendStateCreateInfo colorBlending = Adren::Info::colorBlending();
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &renderer.descriptorSetLayout;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    vibeCheck(vkCreatePipelineLayout(renderer.device, &pipelineLayoutInfo, nullptr, &renderer.pipelineLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = renderer.pipelineLayout;
    pipelineInfo.renderPass = renderer.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    vibeCheck(vkCreateGraphicsPipelines(renderer.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &renderer.graphicsPipeline));

    vkDestroyShaderModule(renderer.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(renderer.device, vertShaderModule, nullptr);
}

void Adren::Pipeline::createDescriptorSetLayout(uint32_t models) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = Adren::Info::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutBinding dynamicUboLayoutBinding = Adren::Info::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1);

    VkDescriptorSetLayoutBinding samplerLayoutBinding = Adren::Info::samplerLayoutBinding();

    std::cout << "Texture Layout Binding Descriptor Count: " << models << "\n \n";
    VkDescriptorSetLayoutBinding textureLayoutBinding = Adren::Info::textureLayoutBinding(models);

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {uboLayoutBinding, dynamicUboLayoutBinding, samplerLayoutBinding, textureLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    vibeCheck(vkCreateDescriptorSetLayout(renderer.device, &layoutInfo, nullptr, &renderer.descriptorSetLayout));
}

void Adren::Pipeline::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1000;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[1].descriptorCount = 1000;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[2].descriptorCount = 1000;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[3].descriptorCount = 1000;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(renderer.swapChainImages.size());
 
    vibeCheck(vkCreateDescriptorPool(renderer.device, &poolInfo, nullptr, &renderer.descriptorPool));
}

void Adren::Pipeline::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(renderer.swapChainImages.size(), renderer.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(renderer.swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    renderer.descriptorSets.resize(renderer.swapChainImages.size());
    vibeCheck(vkAllocateDescriptorSets(renderer.device, &allocInfo, renderer.descriptorSets.data()));
    

    for (size_t i = 0; i < renderer.descriptorSets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = renderer.uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo dynamicBufferInfo{};
        dynamicBufferInfo.buffer = renderer.dynamicUniformBuffers[i];
        dynamicBufferInfo.offset = 0;
        dynamicBufferInfo.range = sizeof(UboDynamicData);

        VkDescriptorImageInfo samplerInfo{};
        samplerInfo.sampler = renderer.textureSampler;

        VkDescriptorImageInfo imageInfo[renderer.textures.size()];
        for (uint32_t f = 0; f < renderer.textures.size(); f++) {
            imageInfo[f].sampler = renderer.textureSampler;     
            imageInfo[f].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[f].imageView = renderer.textures[f].textureImageView;
        }

        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = renderer.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = renderer.descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &dynamicBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = renderer.descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &samplerInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = renderer.descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        std::cout << "Descriptor Write Descriptor Count: " << renderer.textures.size() << "\n \n";
        descriptorWrites[3].descriptorCount = renderer.textures.size(); 
        descriptorWrites[3].pImageInfo = imageInfo;

        vkUpdateDescriptorSets(renderer.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
