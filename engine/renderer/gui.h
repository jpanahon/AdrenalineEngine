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
#include "pipeline.h"
#include "descriptor.h"
#include "renderpass.h"

namespace Adren {
class GUI {
public:
    GUI(Devices& devices, Buffers& buffers, Images& images, Descriptor& descriptor, Swapchain& swapchain, VkInstance& instance, Camera& camera, Config& config) : devices(devices), 
        buffers(buffers), descriptor(descriptor), images(images), swapchain(swapchain), instance(instance), camera(camera), config(config) {}

    void initImGui(GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window);
    void newImguiFrame(GLFWwindow* window);
    void startGUI();
    void recordGUI(VkCommandBuffer& buffer, Buffers& buffers, VkPipeline& pipeline, VkPipelineLayout& layout, uint32_t& index);

    struct Base {
        int32_t width = 1600;
        int32_t height = 1050;
        VkRenderPass renderpass;
        VkCommandPool commandPool;
        Image color, depth;
        VkFramebuffer framebuffer;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet set;
        VkImageView view;
        VkSampler sampler;
    } base;

private:
    void cameraInfo(bool* open);
    void renderInfo(bool* open);
    void viewport();
    void guiStyle();
    void createCommands();
    void createRenderPass();
    void createFramebuffers();
    VkDescriptorSet vport;

    Devices& devices;
    Buffers& buffers;
    Descriptor& descriptor;
    Swapchain& swapchain;
    Images& images;

    Config& config;
    Camera& camera;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDevice& device = devices.device;
    VkInstance& instance;
    VkQueue& graphicsQueue = devices.graphicsQueue;
    QueueFamilyIndices queueFam;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;




    bool rightClick = false;

    int savedX = 0;
    int savedY = 0;
    
    bool showRenderInfo = false;
    bool showCameraInfo = false;

    bool modelInfoLoaded = false;
};
}
