/*
    processing.h
    Adrenaline Engine

    This handles the declaration of the buffers.
*/

#pragma once
#include "swapchain.h"

namespace Adren {
class Processing {
public:
    Processing(Swapchain& swapchain, Camera& camera) : swapchain(swapchain), camera(camera) {}
    void createVertexBuffer();
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createIndexBuffer();
    void createCommands(VkSurfaceKHR& surface);
    void updateUniformBuffer();
    void updateDynamicUniformBuffer(uint32_t currentImage);
    void displayModels();
    void createSyncObjects();
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void loadTextures(std::vector<Texture>& textures);
    void drawFrame();
    void cleanup();
    
    Swapchain& swapchain;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDevice& device = swapchain.devices.device;
    VkPhysicalDevice& physicalDevice = swapchain.devices.physicalDevice;
    VkQueue& graphicsQueue = swapchain.devices.graphicsQueue;
    VkQueue& presentQueue = swapchain.devices.presentQueue;

    GLFWwindow* window = swapchain.window;
    Camera& camera;
private:

    Config& config = swapchain.config;
    size_t currentFrame = 0;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<VkDeviceSize> indexCounts;
    std::vector<VkDeviceSize> vertexCounts;

    Buffer vertexBuffer;
    
    Buffer indexBuffer;

    VmaAllocator& allocator = swapchain.allocator;

    static constexpr int maxFramesInFlight = 3;

    Frame frames[maxFramesInFlight];
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
};
}
