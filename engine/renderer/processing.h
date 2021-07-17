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
    Processing(Swapchain& swapchain, VkDevice& device, VkPhysicalDevice& physicalDevice, std::vector<Model>& models, VkQueue& graphicsQueue, VkQueue& presentQueue, VkSurfaceKHR& surface, GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp, bool& framebufferResized) : swapchain(swapchain), device(device), physicalDevice(physicalDevice), 
        models(models), graphicsQueue(graphicsQueue), presentQueue(presentQueue), surface(surface), window(window), 
        cameraPos(cameraPos), cameraFront(cameraFront), cameraUp(cameraUp), framebufferResized(framebufferResized) {}

    ~Processing();
    
    void createVertexBuffer();

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createIndexBuffer();

    void createCommandPool();

    void createCommandBuffers();

    void createUniformBuffers();

    void updateUniformBuffer(uint32_t currentImage);

    void createDynamicUniformBuffers();

    void updateDynamicUniformBuffer(uint32_t currentImage);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    Texture createTextureImage(std::string TEXTURE_PATH);
    
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);

    void displayModels();
    
    void createSyncObjects();
    
    void recreateSwapChain(std::vector<Texture> textures);

    void drawFrame(std::vector<Texture> textures);
    
    VkCommandPool commandPool;
private:
    VkDevice& device;
    VkPhysicalDevice& physicalDevice;
    VkSurfaceKHR& surface;
    VkQueue& graphicsQueue;
    VkQueue& presentQueue;
    bool& framebufferResized;
    GLFWwindow* window;
    glm::vec3& cameraPos;
    glm::vec3& cameraFront;
    glm::vec3& cameraUp;

    Swapchain& swapchain;
    std::vector<Model>& models;
    size_t currentFrame = 0;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::vector<uint32_t> indexCounts;
    std::vector<uint32_t> vertexCounts;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    int maxFramesInFlight;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
};
}
