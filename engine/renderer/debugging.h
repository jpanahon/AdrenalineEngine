/*  
	debugging.h
	Adrenaline Engine
	
	Declarations for debugging functions.
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Adren {
class Debugger {
public:
    
    Debugger(bool& debug, VkInstance& instance) : debug(debug), instance(instance) {}

    ~Debugger() { if (debug) { DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr); } }
    
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger); 
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator); 
    
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    
    void setupDebugMessenger();
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
private:
    bool& debug;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkInstance& instance;
}; 
}
