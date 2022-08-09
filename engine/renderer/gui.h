/*  
	gui.h
	Adrenaline Engine
	
	This header file has the definitions for the class that handles the graphical user interface.
    It depends on the device, buffer, images, and swapchain functions.
*/

#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <vector>
#include "types.h"
#include "swapchain.h"
#include "pipeline.h"
#include "renderpass.h"
#include "camera.h"

namespace Adren {
class GUI {
public:
    GUI(Devices& devices, Buffers& buffers, Images& images, Swapchain& swapchain, VkInstance& instance, Camera& camera) : 
        buffers(buffers), images(images), swapchain(swapchain), instance(instance), camera(camera),
        device(devices.device), graphicsQueue(devices.graphicsQueue), gpu(devices.gpu), allocator(devices.allocator) {}

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

    ImGuiContext* ctx = nullptr;
    ImGuiStyle* style = nullptr;
private:
    void createCommands();
    void createRenderPass();
    void createFramebuffers();
    void resize();
    
    Buffers& buffers;
    Swapchain& swapchain;
    Images& images;

    Camera& camera;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDevice& device;
    VkInstance& instance;
    VkQueue& graphicsQueue;
    QueueFamilyIndices queueFam;
    VkPhysicalDevice& gpu;
    VmaAllocator& allocator;

    bool rightClick = false;

    int savedX = 0;
    int savedY = 0;

    bool modelInfoLoaded = false;
};
}
