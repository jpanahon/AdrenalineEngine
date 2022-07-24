/*
    processing.h
    Adrenaline Engine

    This handles the declaration of the buffers.
*/

#pragma once
#include "gui.h"
#include "descriptor.h"

namespace Adren {
class Processing {
public:
    Processing(Devices& devices, Camera& camera, std::vector<Model>& models, GLFWwindow* window) :
        device(devices.device), camera(camera), models(models), window(window), gpu(devices.gpu),
        graphicsQueue(devices.graphicsQueue), presentQueue(devices.presentQueue) {}

    void createCommands(VkSurfaceKHR& surface, VkInstance& instance);
    void createSyncObjects();
    void render(Buffers& buffers, Pipeline& pipeline, Descriptor& descriptor, Swapchain& swapchain, Renderpass& renderpass, GUI& gui);
    void cleanup();
   
    VkCommandPool commandPool = VK_NULL_HANDLE;
    size_t currentFrame = 0;
private:
    GLFWwindow* window;
    Camera& camera;
    std::vector<Model>& models;
    VkDevice& device;
    VkPhysicalDevice& gpu;
    VkQueue& graphicsQueue;
    VkQueue& presentQueue;
    std::vector<VkCommandBuffer> commandBuffers;

    static const int maxFramesInFlight = 3;

    Frame frames[maxFramesInFlight];
};
}
