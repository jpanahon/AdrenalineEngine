/*
    processing.h
    Adrenaline Engine

    This handles the declaration of the buffers.
*/

#pragma once
#include "gui.h"


namespace Adren {
class Processing {
public:
    Processing(Devices& devices, Camera& camera, Config& config, GLFWwindow* window) : devices(devices), camera(camera), config(config), window(window) {}
    void createCommands(VkSurfaceKHR& surface, VkInstance& instance);
    void createSyncObjects();
    void render(Buffers& buffers, Pipeline& pipeline, Descriptor& descriptor, Swapchain& swapchain, Renderpass& renderpass, GUI& gui);
    void cleanup();
   
    VkCommandPool commandPool = VK_NULL_HANDLE;
    size_t currentFrame = 0;
private:
    GLFWwindow* window;
    Camera& camera;
    Config& config;
    Devices& devices;
    VkDevice& device = devices.device;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;
    VkQueue& graphicsQueue = devices.graphicsQueue;
    VkQueue& presentQueue = devices.presentQueue;
    std::vector<VkCommandBuffer> commandBuffers;

    static const int maxFramesInFlight = 3;

    Frame frames[maxFramesInFlight];
};
}
