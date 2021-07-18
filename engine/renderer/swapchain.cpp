/* 
	swapchain.cpp
	Adrenaline Engine

	This has the definitions of the functions related to the swapchain.
*/

#include "swapchain.h"
#include "tools.h"
#include "info.h"

VkSurfaceFormatKHR Adren::Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    
    return availableFormats[0];
}

VkExtent2D Adren::Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
            
        };
        
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        
        return actualExtent;
    }
}

void Adren::Swapchain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    vibeCheck(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain));
    
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Adren::Swapchain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    
    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Adren::Swapchain::createRenderPass() {
    VkAttachmentDescription colorAttachment = ::colorAttachment(swapChainImageFormat);
    VkAttachmentDescription depthAttachment = ::depthAttachment(findDepthFormat(physicalDevice));
    

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = ::dependency();

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vibeCheck(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

VkPresentModeKHR Adren::Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

void Adren::Swapchain::cleanupSwapChain() {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

VkImage Adren::Swapchain::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image;
    vibeCheck(vkCreateImage(device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

    vibeCheck(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));
    vkBindImageMemory(device, image, imageMemory, 0);

    return image;
}

VkImageView Adren::Swapchain::createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    VkImageView imageView;
    vibeCheck(vkCreateImageView(device, &viewInfo, nullptr, &imageView));
    
    return imageView;
}

void Adren::Swapchain::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        vibeCheck(vkCreateFramebuffer(device, &framebufferInfo, nullptr, 
            &swapChainFramebuffers[i]));
    }
}

void Adren::Swapchain::createGraphicsPipeline() {
    auto vertShaderCode = readFile("engine/resources/shaders/vert.spv");
    auto fragShaderCode = readFile("engine/resources/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = ::vertShaderStageInfo();
    vertShaderStageInfo.module = vertShaderModule;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = ::fragShaderStageInfo();
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

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = ::inputAssembly();
    
    VkViewport viewport = ::viewport();
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;

    VkRect2D scissor = ::scissor();
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = ::viewportState();
    viewportState.pViewports = &viewport;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = ::rasterizer();

    VkPipelineMultisampleStateCreateInfo multisampling = ::multisampling();
    
    VkPipelineDepthStencilStateCreateInfo depthStencil = ::depthStencil();

    VkPipelineColorBlendAttachmentState colorBlendAttachment = ::colorBlendAttachment();

    VkPipelineColorBlendStateCreateInfo colorBlending = ::colorBlending();
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(int);
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    vibeCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

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
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    vibeCheck(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Adren::Swapchain::createDescriptorSetLayout(uint32_t models) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = ::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutBinding dynamicUboLayoutBinding = ::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1);

    VkDescriptorSetLayoutBinding samplerLayoutBinding = ::samplerLayoutBinding();

    VkDescriptorSetLayoutBinding textureLayoutBinding = ::textureLayoutBinding(models);

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {uboLayoutBinding, dynamicUboLayoutBinding, samplerLayoutBinding, textureLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    vibeCheck(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void Adren::Swapchain::createDescriptorPool() {
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
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
 
    vibeCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
}

void Adren::Swapchain::createDescriptorSets(std::vector<Texture>& textures) {
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChainImages.size());
    vibeCheck(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()));
    

    for (size_t i = 0; i < descriptorSets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo dynamicBufferInfo{};
        dynamicBufferInfo.buffer = dynamicUniformBuffers[i];
        dynamicBufferInfo.offset = 0;
        dynamicBufferInfo.range = sizeof(UboDynamicData);
        
        VkSamplerCreateInfo sampInfo = ::samplerInfo();
        vibeCheck(vkCreateSampler(device, &sampInfo, nullptr, &sampler));

        VkDescriptorImageInfo samplerInfo{};
        samplerInfo.sampler = sampler;

        VkDescriptorImageInfo imageInfo[textures.size()];
        for (uint32_t f = 0; f < textures.size(); f++) {
            imageInfo[f].sampler = sampler;     
            imageInfo[f].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[f].imageView = textures[f].textureImageView;
        }

        std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &dynamicBufferInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &samplerInfo;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrites[3].descriptorCount = textures.size(); 
        descriptorWrites[3].pImageInfo = imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Adren::Swapchain::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(swapChainImages.size());
    uniformBuffersMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i], device, physicalDevice);
    }
}
void Adren::Swapchain::createDynamicUniformBuffers(uint32_t count) {
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkDeviceSize minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    dynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    VkDeviceSize bufferSize = count * dynamicAlignment;
    uboDynamicData.model = (glm::mat4*)malloc(bufferSize);
    assert(uboDynamicData.model);

    dynamicUniformBuffers.resize(swapChainImages.size());
    dynamicUniformBuffersMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,dynamicUniformBuffers[i], dynamicUniformBuffersMemory[i], device, physicalDevice);
    }
}

void Adren::Swapchain::createDepthResources() {
    VkFormat depthFormat = findDepthFormat(physicalDevice);

    depthImage = createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}