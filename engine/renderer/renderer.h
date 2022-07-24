/*
    renderer.h
    Adrenaline Engine
     
    This has the function and class definitions for the renderer.
*/

#pragma once
#include <iostream>
#include <vector>

#include "config.h"
#include "model.h"
#include "camera.h"
#include "debugging.h"
#include "processing.h" // Has all the other components included

namespace Adren {
class Renderer {
public:
    Renderer(GLFWwindow* window) : window(window) {}

    void init(GLFWwindow* window);
    void cleanup();
    void process(GLFWwindow* window);
    void reloadScene(std::vector<Model>& models);
    void wait() { vkDeviceWaitIdle(devices.device); }
    void addModel(std::string& path);
    Camera camera;
    std::vector<Model> models;
    GUI gui{devices, buffers, images, swapchain, instance, camera}; 
private:
    void createInstance();
    void initVulkan();
    void processInput(GLFWwindow* window, Camera& camera);
    std::vector<Model::Texture> textures;
    
    VkInstance instance;
    VkSurfaceKHR surface;
    GLFWwindow* window;
    float lastFrame = 0.0f;

    Devices devices{instance, surface};
#ifdef DEBUG
    Debugger debugging{config.debug, instance};
#endif
    Buffers buffers{devices};
    Swapchain swapchain{devices, window};
    Images images{models, devices, buffers};
    Renderpass renderpass{devices};
    Descriptor descriptor{devices, buffers};
    Pipeline pipeline{devices};
    Processing processing{devices, camera, models, window};
};
}
