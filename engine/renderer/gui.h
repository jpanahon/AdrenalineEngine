/*  
	gui.h
	Adrenaline Engine
	
	This header file has the definitions for the class that handles the graphical user interface.
    It depends on the device, buffer, images, and swapchain functions.
*/

#pragma once
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
    GUI(Devices& devices, Buffers& buffers, Images& images, Swapchain& swapchain, VkInstance& instance, Camera& camera, Config& config) : devices(devices), 
        buffers(buffers), images(images), swapchain(swapchain), instance(instance), camera(camera), config(config) {}

    void init(GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window);
    void newFrame(GLFWwindow* window);
    void viewport();
    void beginRenderpass(VkCommandBuffer& buffer, VkPipeline& pipeline, Buffer vertex, Buffer index);

    struct Base {
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
    
    void createCommands();
    void createRenderPass();
    void createFramebuffers();
    void resize();
    

    Devices& devices;
    Buffers& buffers;
    Descriptor& descriptor;
    Swapchain& swapchain;
    Images& images;

    Camera& camera;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDevice& device = devices.device;
    VkInstance& instance;
    VkQueue& graphicsQueue = devices.graphicsQueue;
    QueueFamilyIndices queueFam;
    VkPhysicalDevice& gpu = devices.gpu;

    bool rightClick = false;

    int savedX = 0;
    int savedY = 0;
    
    bool showRenderInfo = false;
    bool showCameraInfo = false;

    bool modelInfoLoaded = false;
};
}
