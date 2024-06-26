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
    GUI(Devices* devices, Buffers& buffers, Images& images, Swapchain& swapchain, VkInstance& instance) : buffers(buffers), images(images), swapchain(swapchain), 
        instance(instance), device(devices->getDevice()), graphicsQueue(devices->getGraphicsQ()), gpu(devices->getGPU()), allocator(devices->getAllocator()) {}

    void init(Camera& camera, GLFWwindow* window, VkSurfaceKHR& surface);
    void cleanup();
    void mouseHandler(GLFWwindow* window, Camera& camera);
    void newFrame(GLFWwindow* window, Camera& camera);
    void viewport(Camera& camera);
    void beginRenderpass(Camera& camera, VkCommandBuffer& buffer, VkPipeline& pipeline, Buffer& vertex, Buffer& index);
    void draw(VkCommandBuffer& commandBuffer);

    struct Base {
        VkRenderPass renderpass;
        VkCommandPool commandPool;
        Image color, depth;
        VkFramebuffer framebuffer;
        VkDescriptorSet set;
        VkImageView view;
        VkSampler sampler;
    } base{};

    ImGuiContext* ctx = nullptr;
    ImGuiStyle* style = nullptr;
private:
    void createCommands();
    void createSampler();
    void createRenderPass(Camera& camera);
    void createFramebuffers(Camera& camera);
    void resize(ImVec2& size, Camera& camera);
    void createDescriptorPool();
    
    Swapchain& swapchain;
    Images& images;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDevice& device;
    VkInstance& instance;
    VkQueue& graphicsQueue;
    QueueFamilyIndices queueFam;
    VkPhysicalDevice& gpu;
    VmaAllocator& allocator;
};
}
