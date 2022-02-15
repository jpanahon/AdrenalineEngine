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
#include "tools.h"

namespace Adren {
class Swapchain {
public:
	Swapchain(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, 
        GLFWwindow* window, int& modelCount, VmaAllocator& allocator) : device(device), physicalDevice(physicalDevice), surface(surface),
        window(window), modelCount(modelCount), allocator(allocator) {}

	void cleanup() { 
        vkDestroyImage(device, depthImage, nullptr);
        vkDestroyImageView(device, depthImageView, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroySampler(device, sampler, nullptr); 

        for (int i = 0; i < swapChainImages.size(); i++) {
            vmaDestroyBuffer(allocator, uniformBuffers[i].buffer, uniformBuffers[i].alloc);
            // vmaFreeMemory(allocator, uniformBuffers[i].alloc);

            vmaDestroyBuffer(allocator, dynamicUniformBuffers[i].buffer, uniformBuffers[i].alloc);
            // vmaFreeMemory(allocator, dynamicUniformBuffers[i].alloc);
        }
    }

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device);
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice& physicalDevice);
    VkFormat findDepthFormat(VkPhysicalDevice& physicalDevice);
    void createImageViews();
    void createRenderPass();
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	void createFramebuffers();
	void createGraphicsPipeline();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets(std::vector<Texture>& textures);
    void createUniformBuffers();
    void createDynamicUniformBuffers();
    void createDepthResources();

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    std::vector<Buffer> uniformBuffers;
    std::vector<Buffer> dynamicUniformBuffers;

    UboDynamicData uboDynamicData;
    VkDeviceSize dynamicAlignment;

    VmaAllocator& allocator;
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

    VkSampler sampler;
    int& modelCount;
};
}