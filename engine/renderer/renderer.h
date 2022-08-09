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

#ifdef DEBUG
    #include "debugging.h"
#endif

#include "processing.h" // Has all the other components included
#include "descriptor.h"

namespace Adren {
class Renderer {
public:
    void init(GLFWwindow* window);
    void cleanup();
    void process(GLFWwindow* window);
    void reloadScene(std::vector<Model>& models);
    void wait() { vkDeviceWaitIdle(devices.device); }
    void addModel(std::string& path);
    Camera camera;
    std::vector<Model> models = { Model("C:\\Users\\jovic\\Documents\\Coding\\AdrenalineEngine\\engine\\resources\\models\\sponza\\Sponza.gltf") };
    GUI gui{devices, buffers, images, swapchain, instance, camera}; 
private:
    void createInstance();
    void initVulkan(GLFWwindow* window);
    void processInput(GLFWwindow* window, Camera& camera);
    std::vector<Model::Texture> textures;
    
    VkInstance instance;
    VkSurfaceKHR surface;
    GLFWwindow* window;
    float lastFrame = 0.0f;

    Devices devices{instance, surface};
#ifdef DEBUG
    Debugger debugging{instance};
#endif
    Buffers buffers{devices};
    Swapchain swapchain{devices, window};
    Images images{devices, buffers};
    Renderpass renderpass{devices};
    Descriptor descriptor{devices, buffers};
    Pipeline pipeline{devices};
    Processing processing{devices, camera, window};
};
}
