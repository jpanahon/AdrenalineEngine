/*
	devices->h
	Adrenaline Engine
	
	Definitions for the devices class
*/

#pragma once
#include <vector>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vk_mem_alloc.h"

namespace Adren {
class Devices {
public:
    Devices(VkInstance& instance, VkSurfaceKHR& surface) : instance(instance), surface(surface) {
        pickGPU(surface); createLogicalDevice(); createAllocator();
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct Buffer {
        VkBuffer buffer;
        VmaAllocation memory;
        VkDeviceSize size = 0;
        VkDeviceSize align = 0;
        void* mapped;
    };
    
    void init(VkSurfaceKHR& surface) {  }
    void cleanup() { vmaDestroyAllocator(allocator); vkDestroyDevice(device, nullptr);  }
    std::vector<const char*> getRequiredExtensions();
    SwapChainSupportDetails querySwapChainSupport();

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandPool);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice& card);
    void* alignedAlloc(size_t size, size_t alignment);
    void alignedFree(void* data);

#ifdef ADREN_DEBUG
    bool checkDebugSupport();
#endif

    VkDevice& getDevice() { return device;  }
    VkPhysicalDevice& getGPU() { return gpu; }
    VmaAllocator& getAllocator()  { return allocator; }
    const std::vector<const char*>& getDebugLayers() const { return validationLayers;  }
    VkQueue& getPresentQ() { return presentQueue; }
    VkQueue& getGraphicsQ() { return graphicsQueue; }

private:
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
    VkInstance& instance;
    VkSurfaceKHR& surface;
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    
    void pickGPU(VkSurfaceKHR& surface);
    void createLogicalDevice();
    void createAllocator();
    bool isDeviceSuitable(VkPhysicalDevice& card);
    bool checkDeviceExtensionSupport(VkPhysicalDevice& device);
};
}
