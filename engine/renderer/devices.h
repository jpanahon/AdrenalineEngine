/*
	devices.h
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
    Devices(bool& debug, VkInstance& instance, VkSurfaceKHR& surface) : debug(debug), instance(instance), surface(surface) {}

    void cleanup() { vkDestroyDevice(device, nullptr); vmaDestroyAllocator(allocator); }
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createAllocator();
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR& surface;
    VmaAllocator allocator = VK_NULL_HANDLE;
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
private:
    bool& debug;
    VkInstance& instance;
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice& device);

};
}
