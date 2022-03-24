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
    GUI(Devices& devices, Buffers& buffers, Swapchain& swapchain, Processing& processing, Renderpass& renderpass, VkInstance& instance, Camera& camera, Config& config) : devices(devices), 
        buffers(buffers), swapchain(swapchain), processing(processing), renderpass(renderpass), instance(instance), camera(camera), config(config) {}

    void initImGui(GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window);
    void newImguiFrame(GLFWwindow* window);
    void startGUI();
private:
    void cameraInfo(bool* open);
    void renderInfo(bool* open);
    void guiStyle();

    Devices& devices;
    Buffers& buffers;
    Swapchain& swapchain;
    Processing& processing;
    Renderpass& renderpass;
    Config& config;
    Camera& camera;

    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    VkDevice& device = devices.device;
    VkInstance& instance;
    VkQueue& graphicsQueue = devices.graphicsQueue;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;
    VkRenderPass& renderPass = renderpass.handle;
    VkCommandPool& commandPool = processing.commandPool;

    bool rightClick = false;

    int savedX = 0;
    int savedY = 0;
    
    bool showRenderInfo = false;
    bool showCameraInfo = false;
};
}
