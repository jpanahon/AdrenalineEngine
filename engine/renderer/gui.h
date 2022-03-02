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
    GUI(Processing& processing, VkInstance& instance) : processing(processing), instance(instance) {}

    void initImGui(GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window);
    void newImguiFrame(GLFWwindow* window);
    void startGUI();
private:
    void cameraInfo();
    void guiStyle();

    Processing& processing;
    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    VkDevice& device = processing.device;
    VkInstance& instance;
    VkQueue& graphicsQueue = processing.graphicsQueue;
    VkPhysicalDevice& physicalDevice = processing.physicalDevice;
    VkRenderPass& renderPass = processing.swapchain.renderPass;
    std::vector<VkImage>& swapChainImages = processing.swapchain.swapChainImages;
    VkCommandPool& commandPool = processing.commandPool;
    Camera& camera = processing.camera;

    bool rightClick = false;
};
}
