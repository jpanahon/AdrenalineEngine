/*
	devices->h
	Adrenaline Engine
	
	Definitions for the devices class
*/

#pragma once
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vk_mem_alloc.h"

namespace Adren {
class Devices {
public:
    Devices(VkInstance& instance, VkSurfaceKHR& surface) : instance(instance), surface(surface) {}

    void init(VkSurfaceKHR& surface) { pickGPU(surface); createLogicalDevice(); createAllocator(); }
    void cleanup() { vmaDestroyAllocator(getAllocator()); vkDestroyDevice(device, nullptr);  }
    std::vector<const char*> getRequiredExtensions();
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
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkPhysicalDevice gpu = VK_NULL_HANDLE;  
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    void pickGPU(VkSurfaceKHR& surface);
    void createLogicalDevice();
    void createAllocator();
    bool isDeviceSuitable(VkPhysicalDevice& device, VkSurfaceKHR& surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice& device);

};
}
