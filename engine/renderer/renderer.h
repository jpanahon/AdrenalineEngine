/*
    renderer.h
    Adrenaline Engine
     
    This has the function and class definitions for the renderer.
*/

#pragma once
#include <iostream>
#include <vector>

#include "model.h"
#include "camera.h"

#ifdef ADREN_DEBUG
    #include "debugger.h"
#endif

#include "gui.h"
#include "descriptor.h"

namespace Adren {
class Renderer {
public:
    void init(GLFWwindow* window, Camera& camera);
    void cleanup(Camera& camera);
    void render(Camera& camera);
    void reloadScene(std::vector<Model*>& models, Camera& camera);
    void wait() { vkDeviceWaitIdle(devices->getDevice()); }
    void addModel(char* path);
    void processInput(GLFWwindow* window, Camera& camera);
    //Model* cubes = new Model("../engine/resources/models/deccer/cubes.gltf");
    Model* sponza = new Model("../engine/resources/models/sponza/sponza.gltf");
    std::vector<Model*> models = { sponza };
    Devices* devices = new Devices{instance, surface};
    GUI gui{devices, buffers, images, swapchain, instance};

    void createInstance();
    void initVulkan(GLFWwindow* window, Camera& camera);
    void createCommands();
    void createSyncObjects();
    std::vector<Model::Texture> textures;

    static const int maxFramesInFlight = 3;

    std::array<Frame, maxFramesInFlight> frames;
    VkInstance instance;
    VkSurfaceKHR surface;
    
    float lastFrame = 0.0f;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    size_t currentFrame = 0;
    size_t objects = models.size();
    
#ifdef ADREN_DEBUG
    // This sets up Vulkan validation layers.

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    void fillDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    VkResult createDebugUtils(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger
    );

    void destroyDebugUtils(
        VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator
    );

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, 
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
        void* pUserData
    );

    void setupDebugger();
#endif

    Buffers buffers{instance, devices};
    Swapchain swapchain{devices};
    Images images{devices, buffers};
    Renderpass renderpass{devices};
    Descriptor descriptor{devices, buffers};
    Pipeline pipeline{devices};
};
}
