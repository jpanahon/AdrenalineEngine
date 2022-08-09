/*
    processing.cpp
    Adrenaline Engine
    
    This processes everything.
*/

#include "processing.h"
#include <cmath>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

void Adren::Processing::cleanup() {
    vkDestroyCommandPool(device, commandPool, nullptr);
    for (size_t i = 0; i < maxFramesInFlight; i++) {
        vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
        vkDestroySemaphore(device, frames[i].rSemaphore, nullptr);
        vkDestroySemaphore(device, frames[i].iSemaphore, nullptr);
        vkDestroyFence(device, frames[i].fence, nullptr);
    }
}

void Adren::Processing::createCommands(VkSurfaceKHR& surface, VkInstance& instance) {
    QueueFamilyIndices queueFamilyIndices = Adren::Tools::findQueueFamilies(gpu, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    for (int i = 0; i < maxFramesInFlight; i++) {
        Adren::Tools::vibeCheck("COMMAND POOL", vkCreateCommandPool(device, &poolInfo, nullptr, &frames[i].commandPool));
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = frames[i].commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        Adren::Tools::vibeCheck("ALLOCATE COMMAND BUFFERS", vkAllocateCommandBuffers(device, &allocInfo, &frames[i].commandBuffer));
        
#ifdef DEBUG
            Adren::Tools::label(instance, device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)frames[i].commandPool, "FRAME COMMAND POOL");
            Adren::Tools::label(instance, device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)frames[i].commandBuffer, "FRAME COMMAND BUFFER");
#endif
    }

    VkCommandPoolCreateInfo primaryPoolInfo{};
    primaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    primaryPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    primaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
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

void Adren::Processing::render(std::vector<Model>& models, Buffers& buffers, Pipeline& pipeline, std::vector<VkDescriptorSet>& sets, Swapchain& swapchain, Renderpass& renderpass, GUI& gui) {
    ImGui::Render();

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

    gui.beginRenderpass(commandBuffer, pipeline.handle, buffers.vertex, buffers.index);
    
    Offset offset = { 0, 0, 0, 0, 0, buffers.dynamicUniform.align };
    if (models.size() >= 1) {
        for (uint32_t m = 0; m < models.size(); m++) {
            for (size_t n = 0; n < models[m].nodes.size(); n++) {
                Model::Node node = models[m].nodes[n];
                models[m].drawNode(commandBuffer, pipeline.layout, node, sets[imageIndex], offset);
            }
            offset.texture += models[m].textures.size();
            offset.dynamic += static_cast<uint32_t>(offset.align);
        }
    }
    vkCmdEndRenderPass(commandBuffer);

    renderpass.begin(commandBuffer, imageIndex, swapchain.framebuffers, swapchain.extent);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {frames[currentFrame].iSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    std::array<VkCommandBuffer, 1> commandBuffers = { commandBuffer };
    submitInfo.commandBufferCount = commandBuffers.size();

    submitInfo.pCommandBuffers = commandBuffers.data();
    
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
