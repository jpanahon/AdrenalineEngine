/*  
	display.h
	Adrenaline Engine
	
	Handles what is shown on screen.
	
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

namespace Adren {
class GUI {
public:
    GUI(VkDevice& device, VkInstance& instance, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue, 
        std::vector<VkImage>& swapChainImages, VkRenderPass& renderPass, VkCommandPool& commandPool, 
        VkSurfaceKHR& surface, GLFWwindow* window) : device(device), instance(instance), physicalDevice(physicalDevice), 
        graphicsQueue(graphicsQueue),swapChainImages(swapChainImages), renderPass(renderPass), 
        commandPool(commandPool), surface(surface), window(window) {}

    ~GUI() { shutDownImGui(); }

    void initImGui();
    void shutDownImGui();
    void newImguiFrame();
    void startGUI();
private:
    VkDescriptorPool imguiPool;
    VkDevice& device;
    VkInstance& instance;
    VkQueue& graphicsQueue;
    VkPhysicalDevice& physicalDevice;
    VkRenderPass& renderPass;
    std::vector<VkImage>& swapChainImages;
    VkCommandPool& commandPool;
    VkSurfaceKHR& surface;
    GLFWwindow* window;
    bool demoWindow = true;
};
}
