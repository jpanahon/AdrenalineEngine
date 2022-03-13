/*  
	display.h
	Adrenaline Engine
	
	Handles what is shown on screen.
	
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "types.h"
#include "processing.h"

namespace Adren {
class GUI {
public:
    GUI(Devices& devices, Buffers& buffers, Swapchain& swapchain, Processing& processing, VkInstance& instance, Camera& camera, Config& config) : devices(devices), 
        buffers(buffers), swapchain(swapchain), processing(processing), instance(instance), camera(camera), config(config) {}

    void initImGui(GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window);
    void newImguiFrame(GLFWwindow* window);
    void startGUI();
private:
    void cameraInfo();
    void renderInfo();
    void guiStyle();

    Devices& devices;
    Buffers& buffers;
    Swapchain& swapchain;
    Processing& processing;
    Config& config;
    Camera& camera;

    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    VkDevice& device = devices.device;
    VkInstance& instance;
    VkQueue& graphicsQueue = devices.graphicsQueue;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;
    VkRenderPass& renderPass = swapchain.renderPass;
    std::vector<VkImage>& swapChainImages = swapchain.images;
    VkCommandPool& commandPool = processing.commandPool;

    bool rightClick = false;
};
}
