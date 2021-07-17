/* 
	swapchain.h
	Adrenaline Engine

	This has the declarations of the functions related to the swapchain.
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "types.h"

namespace Adren {
class Swapchain {
public:
	Swapchain(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, 
        GLFWwindow* window) : device(device), physicalDevice(physicalDevice), surface(surface), window(window) {}

	~Swapchain() { 
        cleanupSwapChain(); 
        
    }
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImageViews();
    void createRenderPass();
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    void recreateSwapChain();
    void cleanupSwapChain();
	void createFramebuffers();
	void createGraphicsPipeline();
    void createDescriptorSetLayout(uint32_t models);
    void createDescriptorPool();
    void createDescriptorSets(std::vector<Texture>& textures);
    void createUniformBuffers();
    void createDynamicUniformBuffers(uint32_t count);
    void createDepthResources();
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    VkPipeline graphicsPipeline;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkBuffer> dynamicUniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<VkDeviceMemory> dynamicUniformBuffersMemory;
    UboDynamicData uboDynamicData;
    VkDeviceSize dynamicAlignment;
private:
	VkDevice& device;
	VkPhysicalDevice& physicalDevice;
	VkSurfaceKHR& surface;
	
    VkFormat swapChainImageFormat;
   
    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorPool descriptorPool;
    GLFWwindow* window;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

};
}