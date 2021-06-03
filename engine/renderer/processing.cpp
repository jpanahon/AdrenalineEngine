/*
    processing.cpp
    Adrenaline Engine
    
    This processes everything.
    Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#include "processing.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "info.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Adren::Info::attributeDescriptions();
    attributeDescriptions[0].offset = offsetof(Vertex, pos);
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
};

void Adren::Processing::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(renderer.vertices[0]) * renderer.vertices.size();
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(renderer.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, renderer.vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(renderer.device, stagingBufferMemory);
    
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.vertexBuffer, renderer.vertexBufferMemory);
    
    copyBuffer(stagingBuffer, renderer.vertexBuffer, bufferSize);
    
    vkDestroyBuffer(renderer.device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.device, stagingBufferMemory, nullptr);
}

void Adren::Processing::createBuffer(VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vibeCheck(vkCreateBuffer(renderer.device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderer.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    vibeCheck(vkAllocateMemory(renderer.device, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(renderer.device, buffer, bufferMemory, 0);
}

void Adren::Processing::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeCommands(commandBuffer);
}

void Adren::Processing::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(renderer.indices[0]) * renderer.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, renderer.indices.data(), (size_t) bufferSize);
    vkUnmapMemory(renderer.device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer.indexBuffer, renderer.indexBufferMemory);

    copyBuffer(stagingBuffer, renderer.indexBuffer, bufferSize);

    vkDestroyBuffer(renderer.device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.device, stagingBufferMemory, nullptr);
}

uint32_t Adren::Processing::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(renderer.physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Adren::Processing::createFramebuffers() {
    renderer.swapChainFramebuffers.resize(renderer.swapChainImageViews.size());

    for (size_t i = 0; i < renderer.swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            renderer.swapChainImageViews[i],
            renderer.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = renderer.swapChainExtent.width;
        framebufferInfo.height = renderer.swapChainExtent.height;
        framebufferInfo.layers = 1;

        vibeCheck(vkCreateFramebuffer(renderer.device, &framebufferInfo, nullptr, 
            &renderer.swapChainFramebuffers[i]));
    }
}

void Adren::Processing::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(renderer.physicalDevice, renderer.surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vibeCheck(vkCreateCommandPool(renderer.device, &poolInfo, nullptr, &renderer.commandPool));
}

void Adren::Processing::createCommandBuffers() {
    renderer.commandBuffers.resize(renderer.MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) renderer.commandBuffers.size();

    vibeCheck(vkAllocateCommandBuffers(renderer.device, &allocInfo, renderer.commandBuffers.data()));
}

void Adren::Processing::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    renderer.uniformBuffers.resize(renderer.swapChainImages.size());
    renderer.uniformBuffersMemory.resize(renderer.swapChainImages.size());

    for (size_t i = 0; i < renderer.swapChainImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, renderer.uniformBuffers[i], renderer.uniformBuffersMemory[i]);
    }
}

void Adren::Processing::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};
    // ubo.view = glm::lookAt(glm::vec3(player.x, player.y + 0.5f, player.z + 2.5f), glm::vec3(player.x, player.y, player.z) + camera.cameraFront, camera.cameraUp);

    ubo.view = glm::lookAt(display.cameraPos, display.cameraPos + display.cameraFront, display.cameraUp);

    // ubo.view = glm::lookAt(player.model.position, player.model.position + player.model.frontPos, player.model.upPos);
    ubo.proj = glm::perspective(glm::radians(90.0f), renderer.swapChainExtent.width / (float) renderer.swapChainExtent.height, 0.1f, 1000.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(renderer.device, renderer.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
       memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(renderer.device, renderer.uniformBuffersMemory[currentImage]);
}

void Adren::Processing::createDynamicUniformBuffers() {
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(renderer.physicalDevice, &physicalDeviceProperties);
    VkDeviceSize minUboAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    renderer.dynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        renderer.dynamicAlignment = (renderer.dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    VkDeviceSize bufferSize = models.size() * renderer.dynamicAlignment;
    uboDynamicData.model = (glm::mat4*)alignedAlloc(bufferSize, renderer.dynamicAlignment);
    assert(uboDynamicData.model);

    renderer.dynamicUniformBuffers.resize(renderer.swapChainImages.size());
    renderer.dynamicUniformBuffersMemory.resize(renderer.swapChainImages.size());

    for (size_t i = 0; i < renderer.swapChainImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, renderer.dynamicUniformBuffers[i], renderer.dynamicUniformBuffersMemory[i]);
    }
}

void Adren::Processing::updateDynamicUniformBuffer(uint32_t currentImage) {
    for (uint32_t i = 0; i < models.size(); i++) {
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboDynamicData.model + (i * renderer.dynamicAlignment)));
        
        if (models[i].player == true ) {
            *modelMat = glm::translate(glm::mat4(1.0f), display.cameraPos);
            *modelMat = glm::rotate(*modelMat, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            *modelMat = glm::scale(*modelMat, glm::vec3(0.5f));
        } else {
            *modelMat = glm::translate(glm::mat4(1.0f), models[i].position);
        }
        
    }

    void* data;
    vkMapMemory(renderer.device, renderer.dynamicUniformBuffersMemory[currentImage], 0, models.size() * renderer.dynamicAlignment, 0, &data);
    memcpy(data, uboDynamicData.model, renderer.dynamicAlignment * models.size());
    
    VkMappedMemoryRange mappedMemoryRange{};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedMemoryRange.memory = renderer.dynamicUniformBuffersMemory[currentImage];
    mappedMemoryRange.size = renderer.dynamicAlignment * models.size();
    vkFlushMappedMemoryRanges(renderer.device, 1, &mappedMemoryRange); 
    vkUnmapMemory(renderer.device, renderer.dynamicUniformBuffersMemory[currentImage]);
}

VkCommandBuffer Adren::Processing::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer.commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderer.device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void Adren::Processing::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(renderer.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderer.graphicsQueue);
    vkFreeCommandBuffers(renderer.device, renderer.commandPool, 1, &commandBuffer);
}

void Adren::Processing::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void Adren::Processing::createDepthResources() {
    VkFormat depthFormat = findDepthFormat(renderer.physicalDevice);

    renderer.depthImage = createImage(renderer.swapChainExtent.width, renderer.swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,renderer.depthImageMemory);
    renderer.depthImageView = createImageView(renderer.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

Texture Adren::Processing::createTextureImage(std::string TEXTURE_PATH) {
    int texWidth, texHeight, texChannels = 0;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(renderer.device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(renderer.device, stagingBufferMemory);

    stbi_image_free(pixels);

    Texture tex;

    tex.texture = createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tex.textureImageMemory);

    transitionImageLayout(tex.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, tex.texture, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(tex.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(renderer.device, stagingBuffer, nullptr);
    vkFreeMemory(renderer.device, stagingBufferMemory, nullptr);

    tex.textureImageView = createImageView(tex.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    
    return tex;
}

VkImage Adren::Processing::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory) {
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
    if (vkCreateImage(renderer.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(renderer.device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(renderer.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(renderer.device, image, imageMemory, 0);

    return image;
}

void Adren::Processing::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    endSingleTimeCommands(commandBuffer);
}

void Adren::Processing::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(renderer.device, &samplerInfo, nullptr, &renderer.textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkSurfaceFormatKHR Adren::Processing::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    
    return availableFormats[0];
}

VkExtent2D Adren::Processing::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        
        glfwGetFramebufferSize(renderer.window, &width, &height);
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
            
        };
        
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        
        return actualExtent;
    }
}

void Adren::Processing::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(renderer.physicalDevice, renderer.surface);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = renderer.surface;
    
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = findQueueFamilies(renderer.physicalDevice, renderer.surface);
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
    
    vibeCheck(vkCreateSwapchainKHR(renderer.device, &createInfo, nullptr, &renderer.swapChain));
    
    vkGetSwapchainImagesKHR(renderer.device, renderer.swapChain, &imageCount, nullptr);
    renderer.swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(renderer.device, renderer.swapChain, &imageCount, renderer.swapChainImages.data());
    
    renderer.swapChainImageFormat = surfaceFormat.format;
    renderer.swapChainExtent = extent;
}

void Adren::Processing::createImageViews() {
    renderer.swapChainImageViews.resize(renderer.swapChainImages.size());
    
    for (uint32_t i = 0; i < renderer.swapChainImages.size(); i++) {
        renderer.swapChainImageViews[i] = createImageView(renderer.swapChainImages[i], renderer.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Adren::Processing::createRenderPass() {
    VkAttachmentDescription colorAttachment = Adren::Info::colorAttachment(renderer.swapChainImageFormat);
    VkAttachmentDescription depthAttachment = Adren::Info::depthAttachment(findDepthFormat(renderer.physicalDevice));
    

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

    VkSubpassDependency dependency = Adren::Info::dependency();

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vibeCheck(vkCreateRenderPass(renderer.device, &renderPassInfo, nullptr, &renderer.renderPass));
}

VkPresentModeKHR Adren::Processing::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    
    return VK_PRESENT_MODE_FIFO_KHR;
}

void Adren::Processing::recreateSwapChain() {
    
    int width = 0, height = 0;
    glfwGetFramebufferSize(renderer.window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(renderer.window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(renderer.device);
    
    cleanupSwapChain();
    
    createSwapChain();
    
    createImageViews();
    
    createRenderPass();
    
    pipeline.createGraphicsPipeline();
    
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    pipeline.createDescriptorPool();
    
    pipeline.createDescriptorSets();
    createCommandBuffers();
}

void Adren::Processing::cleanupSwapChain() {
    vkDestroyImageView(renderer.device, renderer.depthImageView, nullptr);
    vkDestroyImage(renderer.device, renderer.depthImage, nullptr);
    vkFreeMemory(renderer.device, renderer.depthImageMemory, nullptr);
    
    for (auto framebuffer : renderer.swapChainFramebuffers) {
        vkDestroyFramebuffer(renderer.device, framebuffer, nullptr);
    }
    
    vkFreeCommandBuffers(renderer.device, renderer.commandPool, static_cast<uint32_t>(renderer.commandBuffers.size()), renderer.commandBuffers.data());
    
    vkDestroyPipeline(renderer.device, renderer.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(renderer.device, renderer.pipelineLayout, nullptr);
    vkDestroyRenderPass(renderer.device, renderer.renderPass, nullptr);
    
    for (auto imageView : renderer.swapChainImageViews) {
        vkDestroyImageView(renderer.device, imageView, nullptr);
    }
    
    vkDestroySwapchainKHR(renderer.device, renderer.swapChain, nullptr);
    
    for (size_t i = 0; i < renderer.swapChainImages.size(); i++) {
        vkDestroyBuffer(renderer.device, renderer.uniformBuffers[i], nullptr);
        vkFreeMemory(renderer.device, renderer.uniformBuffersMemory[i], nullptr);
        vkDestroyBuffer(renderer.device, renderer.dynamicUniformBuffers[i], nullptr);
        vkFreeMemory(renderer.device, renderer.dynamicUniformBuffersMemory[i], nullptr);
    }
    
    vkDestroyDescriptorPool(renderer.device, renderer.descriptorPool, nullptr);
}

VkImageView Adren::Processing::createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags) {
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
    vibeCheck(vkCreateImageView(renderer.device, &viewInfo, nullptr, &imageView));
    
    return imageView;
}

void Adren::Processing::displayModels() {
    for (auto &model : models) {
        renderer.vertexCounts.push_back(model.vertices.size());
        renderer.indexCounts.push_back(model.indices.size());

        renderer.indices.insert(renderer.indices.end(), model.indices.begin(),  model.indices.end());
        renderer.vertices.insert(renderer.vertices.end(), model.vertices.begin(), model.vertices.end());
    }
}

void Adren::Processing::createSyncObjects() {
    renderer.imageAvailableSemaphores.resize(renderer.MAX_FRAMES_IN_FLIGHT);
    renderer.renderFinishedSemaphores.resize(renderer.MAX_FRAMES_IN_FLIGHT);
    renderer.inFlightFences.resize(renderer.MAX_FRAMES_IN_FLIGHT);
    renderer.imagesInFlight.resize(renderer.swapChainImages.size(), VK_NULL_HANDLE);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < renderer.MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(renderer.device, &semaphoreInfo, nullptr, &renderer.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(renderer.device, &semaphoreInfo, nullptr, &renderer.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(renderer.device, &fenceInfo, nullptr, &renderer.inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Adren::Processing::drawFrame() {
    ImGui::Render();
    renderer.currentFrame = (renderer.currentFrame + 1) % renderer.MAX_FRAMES_IN_FLIGHT;
    vkWaitForFences(renderer.device, 1, &renderer.inFlightFences[renderer.currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(renderer.device, 1, &renderer.inFlightFences[renderer.currentFrame]);

    
    auto commandBuffer = renderer.commandBuffers[renderer.currentFrame];
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(renderer.device, renderer.swapChain, UINT64_MAX, renderer.imageAvailableSemaphores[renderer.currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }


    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vibeCheck(vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
    vibeCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer.renderPass;
    renderPassInfo.framebuffer = renderer.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = renderer.swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.53f, 0.81f, 0.92f, 0.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.graphicsPipeline);

    VkBuffer vertexBuffers[] = {renderer.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, renderer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    uint32_t firstIndex = 0;
    uint32_t vertexOffset = 0;
    for (int j = 0; j < renderer.indexCounts.size(); j++) {
        uint32_t dynamicOffset = j * static_cast<uint32_t>(renderer.dynamicAlignment);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipelineLayout, 0, 1, &renderer.descriptorSets[imageIndex], 1, &dynamicOffset);

        vkCmdPushConstants(commandBuffer, renderer.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), &j);
        vkCmdDrawIndexed(commandBuffer, renderer.indexCounts[j], 1, firstIndex, vertexOffset, 0);

        firstIndex += renderer.indexCounts[j];
        vertexOffset += renderer.vertexCounts[j];
    }
    
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);

    vibeCheck(vkEndCommandBuffer(commandBuffer)); 

    updateUniformBuffer(imageIndex);
    updateDynamicUniformBuffer(imageIndex);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {renderer.imageAvailableSemaphores[renderer.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    VkSemaphore signalSemaphores[] = {renderer.renderFinishedSemaphores[renderer.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    
    vibeCheck(vkQueueSubmit(renderer.graphicsQueue, 1, &submitInfo, renderer.inFlightFences[renderer.currentFrame]));
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {renderer.swapChain};
    presentInfo.swapchainCount = 1; 
    presentInfo.pSwapchains = swapChains;
    
    presentInfo.pImageIndices = &imageIndex;
    
    result = vkQueuePresentKHR(renderer.presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || renderer.framebufferResized) {
        renderer.framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
}