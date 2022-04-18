/*  
	debugging.h
	Adrenaline Engine
	
	Declarations for debugging functions.
*/

#pragma once
#include <GLFW/glfw3.h>

namespace Adren {
class Debugger {
public:
    
    Debugger(bool& debug, VkInstance& instance) : debug(debug), instance(instance) {}

    void cleanup() { if (debug) { destroyUtils(instance, debugMessenger, nullptr); } }
    
    VkResult createUtils(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger); 
    void destroyUtils(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator); 
    
    void fillCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    
    void setup();
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
private:
    bool& debug;
    VkInstance& instance;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
}; 
}
