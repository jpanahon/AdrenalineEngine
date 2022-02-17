/*
    processing.h
    Adrenaline Engine

    This handles the declaration of the buffers.
*/

#pragma once
#include "swapchain.h"
#include "model.h"

namespace Adren {
class Processing {
public:
    Processing(Swapchain& swapchain, VkDevice& device, VkPhysicalDevice& physicalDevice, 
        std::vector<Model>& models, VkQueue& graphicsQueue, VkQueue& presentQueue, 
        VkSurfaceKHR& surface, GLFWwindow* window, Camera& camera, bool& enableGUI, VmaAllocator& allocator) : swapchain(swapchain),
        device(device), physicalDevice(physicalDevice), models(models), graphicsQueue(graphicsQueue), 
        presentQueue(presentQueue), surface(surface), window(window), camera(camera), enableGUI(enableGUI), allocator(allocator) {}
    
    void createVertexBuffer();

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createIndexBuffer();

    void createCommands();

    void updateUniformBuffer(uint32_t currentImage);

    void updateDynamicUniformBuffer(uint32_t currentImage);

    void displayModels();
    
    void createSyncObjects();
    
    //void recreateSwapChain();

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    Texture createTextureImage(std::string TEXTURE_PATH);
    void drawFrame();

    void cleanup();
    
    VkCommandPool commandPool = VK_NULL_HANDLE;
private:
    VkDevice& device;
    VkPhysicalDevice& physicalDevice;
    VkSurfaceKHR& surface;
    VkQueue& graphicsQueue;
    VkQueue& presentQueue;

    GLFWwindow* window;
    Camera& camera;

    Swapchain& swapchain;
    std::vector<Model>& models;
    size_t currentFrame = 0;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<uint32_t> indexCounts;
    std::vector<uint32_t> vertexCounts;

    Buffer vertexBuffer;
    
    Buffer indexBuffer;

    VmaAllocator& allocator;

    static constexpr int maxFramesInFlight = 3;

    Frame frames[maxFramesInFlight];
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;

    bool& enableGUI;
};
}
