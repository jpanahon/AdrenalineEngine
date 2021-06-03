/*
    processing.h
    Adrenaline Engine

    This handles the declaration of the buffers.
    Copyright © 2021  Stole Your Shoes. All rights reserved.
*/

#pragma once
#include "pipeline.h"
#include "model.h"
#include "display.h"
#include "global.h"

namespace Adren {
class Processing {
public:
    Processing(RendererVariables& renderer, Pipeline& pipeline, std::vector<Model>& models, Display& display) : renderer(renderer), pipeline(pipeline), models(models), display(display) { 
        
    }

    ~Processing() {
        if (uboDynamicData.model) {
            alignedFree(uboDynamicData.model);
        }
    }
    
    void createVertexBuffer();

    void createBuffer(VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createIndexBuffer();

    void createCommandPool();

    void createCommandBuffers();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createFramebuffers();

    void createUniformBuffers();

    void updateUniformBuffer(uint32_t currentImage);

    void createDynamicUniformBuffers();

    void updateDynamicUniformBuffer(uint32_t currentImage);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createDepthResources();
    
    Texture createTextureImage(std::string TEXTURE_PATH);
    
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
    
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    void createTextureSampler();
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    
    void createSwapChain();
    
    void createImageViews();
    
    void createRenderPass();
    
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    
    void recreateSwapChain();
    
    void cleanupSwapChain();
    
    VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);

    void displayModels();
    
    void createSyncObjects();
    
    void drawFrame();
private:
    RendererVariables& renderer;
    Pipeline& pipeline;
    std::vector<Model>& models;
    Display& display;

    UboDynamicData uboDynamicData;
    bool frameStarted{false};
};
}
