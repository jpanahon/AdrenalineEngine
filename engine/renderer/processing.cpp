/*
    processing.cpp
    Adrenaline Engine
    
    This processes everything.
*/

#include "processing.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

void Adren::Processing::cleanup() {
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        vkDestroySemaphore(device, frames[i].rSemaphore, nullptr);
        vkDestroySemaphore(device, frames[i].iSemaphore, nullptr);
        vkDestroyFence(device, frames[i].fence, nullptr);
        vkEndCommandBuffer(frames[i].commandBuffer);
        vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
    }
    
    vkDestroyCommandPool(device, commandPool, nullptr);
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

void Adren::Processing::render(Buffers& buffers, Pipeline& pipeline, Descriptor& descriptor, Swapchain& swapchain, Renderpass& renderpass, GUI& gui) {
    if (config.enableGUI) { ImGui::Render(); }

    currentFrame = (currentFrame + 1) % maxFramesInFlight;
    vkWaitForFences(device, 1, &frames[currentFrame].fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frames[currentFrame].fence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain.handle, UINT64_MAX, frames[currentFrame].iSemaphore, VK_NULL_HANDLE, &imageIndex);
    auto commandBuffer = frames[currentFrame].commandBuffer;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    //vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    vkResetCommandPool(device, frames[currentFrame].commandPool, 0);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    renderpass.begin(commandBuffer, imageIndex, swapchain.framebuffers, swapchain.extent);

    VkViewport viewport = { 0, float(swapchain.extent.height), float(swapchain.extent.width), -float(swapchain.extent.height), 0, 1 };
    VkRect2D scissor = { {0, 0}, {uint32_t(swapchain.extent.width), uint32_t(swapchain.extent.height)} };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

    VkBuffer vertexBuffers[] = { buffers.vertex.buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, buffers.index.buffer, 0, VK_INDEX_TYPE_UINT32);
    Offset offset{};
    offset.firstIndex = 0;
    offset.vertexOffset = 0;
    offset.textureOffset = 0;
    offset.dynamicOffset = 0;
    offset.modelOffset = 0;
    offset.dynamicAlignment = buffers.dynamicAlignment;
    for (int m = 0; m < config.models.size(); m++) {
        offset.modelOffset += config.models[m].offset();
        for (size_t n = 0; n < config.models[m].nodes.size(); n++) {
            Model::Node node = config.models[m].nodes[n];
            config.models[m].drawNode(commandBuffer, pipeline.layout, node, descriptor.sets[imageIndex], offset, buffers.dynamicAlignment);
            //offset.dynamicOffset += offset.modelOffset * static_cast<uint32_t>(buffers.dynamicAlignment);
        }
        offset.textureOffset += config.models[m].textures.size();
    }

    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    if (config.enableGUI) { 
        gui.recordGUI(currentFrame, imageIndex);
    }

    buffers.updateDynamicUniformBuffer(imageIndex, config.models);
    
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
    
    VkSwapchainKHR swapChains[] = {swapchain.handle};
    presentInfo.swapchainCount = 1; 
    presentInfo.pSwapchains = swapChains;
    
    presentInfo.pImageIndices = &imageIndex;
    
    vkQueuePresentKHR(presentQueue, &presentInfo);
}
