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
    Processing(Devices& devices, GLFWwindow* window) : device(devices.getDevice()), window(window), gpu(devices.getGPU()),
        graphicsQueue(devices.getGraphicsQ()), presentQueue(devices.getPresentQ()) {}

    
    void render(Camera* camera, std::vector<Model>& models, Buffers& buffers, Pipeline& pipeline, Descriptor& descriptor, Swapchain& swapchain, Renderpass& renderpass, GUI& gui);
    void cleanup();
private:
    GLFWwindow* window;
    VkDevice& device;
    VkPhysicalDevice& gpu;
    VkQueue& graphicsQueue;
    VkQueue& presentQueue;
    std::vector<VkCommandBuffer> commandBuffers;

    static const int maxFramesInFlight = 3;

    std::array<Frame, maxFramesInFlight> frames = {};
};
}
