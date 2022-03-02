/*
    processing.cpp
    Adrenaline Engine
    
    This processes everything.
*/

#include "processing.h"
#include "tools.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb/stb_image.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "info.h"

void Adren::Processing::cleanup() {
    if (swapchain.uboDynamicData.model) { Adren::Tools::alignedFree(swapchain.uboDynamicData.model); }

    vmaDestroyBuffer(allocator, indexBuffer.buffer, indexBuffer.alloc);
    vmaDestroyBuffer(allocator, vertexBuffer.buffer, vertexBuffer.alloc);
    
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        vkDestroySemaphore(device, frames[i].rSemaphore, nullptr);
        vkDestroySemaphore(device, frames[i].iSemaphore, nullptr);
        vkDestroyFence(device, frames[i].fence, nullptr);
        vkEndCommandBuffer(frames[i].commandBuffer);
        vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
    }
    
    vkDestroyCommandPool(device, commandPool, nullptr);
}

void Adren::Processing::createVertexBuffer() {
    std::cerr << "Vertex Size: " << vertices.size() << "\n \n";
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    Buffer stagingBuffer;
    Adren::Tools::createBuffer(allocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);
    
    void* data;
    vmaMapMemory(allocator, stagingBuffer.alloc, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vmaUnmapMemory(allocator, stagingBuffer.alloc);
    
    Adren::Tools::createBuffer(allocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY);

    copyBuffer(stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);
    
    vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.alloc);
}

void Adren::Processing::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);
    
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);
}

void Adren::Processing::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    Buffer stagingBuffer;
    Adren::Tools::createBuffer(allocator,  bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);

    void* data;
    vmaMapMemory(allocator, stagingBuffer.alloc, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
    vmaUnmapMemory(allocator, stagingBuffer.alloc);

    Adren::Tools::createBuffer(allocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, VMA_MEMORY_USAGE_GPU_ONLY);
    copyBuffer(stagingBuffer.buffer, indexBuffer.buffer, bufferSize);

    vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.alloc);
}

void Adren::Processing::createCommands(VkSurfaceKHR& surface) {
    QueueFamilyIndices queueFamilyIndices = Adren::Tools::findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (int i = 0; i < maxFramesInFlight; i++) {
        Adren::Tools::vibeCheck("COMMAND POOL", vkCreateCommandPool(device, &poolInfo, nullptr, &frames[i].commandPool));

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = frames[i].commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        Adren::Tools::vibeCheck("ALLOCATE COMMAND BUFFERS", vkAllocateCommandBuffers(device, &allocInfo, &frames[i].commandBuffer));
    }

    VkCommandPoolCreateInfo primaryPoolInfo{};
    primaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    primaryPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    primaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    Adren::Tools::vibeCheck("CREATED PRIMARY COMMAND POOL", vkCreateCommandPool(device, &primaryPoolInfo, nullptr, &commandPool));
}

void Adren::Processing::updateUniformBuffer() {
    if (camera.toggled) {
        UniformBufferObject ubo{};
        ubo.view = glm::lookAt(camera.pos, camera.pos + camera.front, camera.up);
        ubo.proj = glm::perspective(glm::radians(110.0f), (float) swapchain.swapChainExtent.width / (float) swapchain.swapChainExtent.height, 0.1f, 10000.0f);
        ubo.proj[1][1] *= -1;
        memcpy(swapchain.uniformBuffer.mapped, &ubo, sizeof(ubo));
    }
}

void Adren::Processing::updateDynamicUniformBuffer(uint32_t currentImage) {
    for (uint32_t i = 0; i < config.models.size(); i++) {
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)swapchain.uboDynamicData.model + (i * swapchain.dynamicAlignment)));
        *modelMat = glm::translate(glm::mat4(1.0f), config.models[i].position);
        
        if (config.models[i].rotationAngle != 0.0f) { *modelMat = glm::rotate(*modelMat, glm::radians(config.models[i].rotationAngle), config.models[i].rotationAxis); }

        if (config.models[i].scale != 0.0f) { *modelMat = glm::scale(*modelMat, glm::vec3(config.models[i].scale)); }
       
    }

    memcpy(swapchain.dynamicUniformBuffers[currentImage].mapped, swapchain.uboDynamicData.model, swapchain.dynamicAlignment * config.models.size());
}

void Adren::Processing::displayModels() {
    for (auto &model : config.models) {
        indices.insert(indices.end(), model.indices.begin(),  model.indices.end());
        vertices.insert(vertices.end(), model.vertices.begin(), model.vertices.end());
    }
}

void Adren::Processing::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frames[i].iSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frames[i].rSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &frames[i].fence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Adren::Processing::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);
}

void Adren::Processing::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);

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
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);
}

void Adren::Processing::loadTextures(std::vector<Texture>& textures) {
    for (Model model : config.models) {
        for (size_t t = 0; t < model.textures.size(); t++) {
            Texture texture = model.textures[t];
            Model::Image image = model.images[t];

            Buffer stagingBuffer = {};
            Adren::Tools::createBuffer(allocator, image.bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, VMA_MEMORY_USAGE_CPU_ONLY);

            uint8_t* data;
            vmaMapMemory(allocator, stagingBuffer.alloc, (void**)&data);
            memcpy(data, image.buffer, image.bufferSize);
            vmaUnmapMemory(allocator, stagingBuffer.alloc);
            
            texture.texture = swapchain.createImage(image.width, image.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.textureImageMemory);

            transitionImageLayout(texture.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer.buffer, texture.texture, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
            transitionImageLayout(texture.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.alloc);

            texture.textureImageView = swapchain.createImageView(texture.texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
            textures.push_back(texture);
        }
    }
}

void Adren::Processing::drawFrame() {
    if (config.enableGUI) { ImGui::Render(); }
    currentFrame = (currentFrame + 1) % maxFramesInFlight;
    vkWaitForFences(device, 1, &frames[currentFrame].fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frames[currentFrame].fence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain.swapChain, UINT64_MAX, frames[currentFrame].iSemaphore, VK_NULL_HANDLE, &imageIndex);
    auto commandBuffer = frames[currentFrame].commandBuffer;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    //vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    vkResetCommandPool(device, frames[currentFrame].commandPool, 0);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapchain.renderPass;
    renderPassInfo.framebuffer = swapchain.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchain.swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.119f, 0.181f, 0.254f, 0.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0, float(swapchain.swapChainExtent.height), float(swapchain.swapChainExtent.width), -float(swapchain.swapChainExtent.height), 0, 1 };
    VkRect2D scissor = { {0, 0}, {uint32_t(swapchain.swapChainExtent.width), uint32_t(swapchain.swapChainExtent.height)} };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain.graphicsPipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    uint32_t firstIndex = 0;
    uint32_t vertexOffset = 0;

    for (int j = 0; j < config.models.size(); j++) {
        uint32_t dynamicOffset = j * static_cast<uint32_t>(swapchain.dynamicAlignment);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, swapchain.pipelineLayout, 0, 1,
            &swapchain.descriptorSets[imageIndex], 1, &dynamicOffset);

        for (const Model::Primitive& p : config.models[j].primitives) {
            Texture texture = config.models[j].textures[config.models[j].materials[p.materialIndex].baseColorTextureIndex];
            vkCmdPushConstants(commandBuffer, swapchain.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), &texture.imageIndex);
            vkCmdDrawIndexed(commandBuffer, p.indexCount, 1, firstIndex, vertexOffset, 0);
            firstIndex += p.indexCount;
            vertexOffset += p.vertexCount;
        }
    }

    if (config.enableGUI) { ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer); }
    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    updateDynamicUniformBuffer(imageIndex);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {frames[currentFrame].iSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    VkSemaphore signalSemaphores[] = {frames[currentFrame].rSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, frames[currentFrame].fence);
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {swapchain.swapChain};
    presentInfo.swapchainCount = 1; 
    presentInfo.pSwapchains = swapChains;
    
    presentInfo.pImageIndices = &imageIndex;
    
    vkQueuePresentKHR(presentQueue, &presentInfo);
}
