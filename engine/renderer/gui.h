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
#include "swapchain.h"
#include "buffers.h"
#include "pipeline.h"
#include "descriptor.h"
#include "renderpass.h"

namespace Adren {
class GUI {
public:
    GUI(Devices& devices, Buffers& buffers, Descriptor& descriptor, Swapchain& swapchain, VkInstance& instance, Camera& camera, Config& config) : devices(devices), 
        buffers(buffers), descriptor(descriptor), swapchain(swapchain), instance(instance), camera(camera), config(config) {}

    void initImGui(GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window);
    void newImguiFrame(GLFWwindow* window);
    void startGUI();
    void recordGUI(size_t& frame, uint32_t& index);
private:
    void cameraInfo(bool* open);
    void renderInfo(bool* open);
    void viewport();
    void guiStyle();
    void createCommands();
    void createRenderPass();
    void createFramebuffers();

    Devices& devices;
    Buffers& buffers;
    Descriptor& descriptor;
    Swapchain& swapchain;

    Config& config;
    Camera& camera;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDevice& device = devices.device;
    VkInstance& instance;
    VkQueue& graphicsQueue = devices.graphicsQueue;
    QueueFamilyIndices queueFam;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    bool rightClick = false;

    int savedX = 0;
    int savedY = 0;
    
    bool showRenderInfo = false;
    bool showCameraInfo = false;

    bool modelInfoLoaded = false;
};
}
