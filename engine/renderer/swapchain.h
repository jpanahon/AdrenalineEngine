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
#include "devices.h"

namespace Adren {
class Swapchain {
public:
    Swapchain(Devices& devices, GLFWwindow* window, Config& config, VmaAllocator& allocator) : devices(devices), 
        window(window), config(config), allocator(allocator) {}

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

        vmaDestroyBuffer(allocator, uniformBuffer.buffer, uniformBuffer.alloc);
        vmaUnmapMemory(allocator, uniformBuffer.alloc);

        for (int i = 0; i < swapChainImages.size(); i++) {
            vmaDestroyBuffer(allocator, dynamicUniformBuffers[i].buffer, dynamicUniformBuffers[i].alloc);
            vmaUnmapMemory(allocator, dynamicUniformBuffers[i].alloc);
        }
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain(VkSurfaceKHR& surface);
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
    void createDescriptorSetLayout(std::vector<Model>& models);
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

    std::vector<Buffer> dynamicUniformBuffers;

    Buffer uniformBuffer;

    UboDynamicData uboDynamicData;
    VkDeviceSize dynamicAlignment;

    Devices& devices;
    VmaAllocator& allocator;
    VkDevice& device = devices.device;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;
    GLFWwindow* window;
    Config& config;
private:
    VkFormat swapChainImageFormat;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    VkSampler sampler;
};
}