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
    Renderer(Config& config, GLFWwindow* window) : config(config), window(window) {}

    void init(GLFWwindow* window);
    void cleanup();
    void process(GLFWwindow* window);
    void wait() { vkDeviceWaitIdle(devices.device); }
    Camera camera;
    GUI gui{devices, buffers, images, swapchain, instance, camera, config}; 
private:
    void createInstance();
    void initVulkan();
    void processInput(GLFWwindow* window, Camera& camera);

    Config& config;
    std::vector<Model::Texture> textures;
    
    VkInstance instance;
    VkSurfaceKHR surface;
    GLFWwindow* window;
    float lastFrame = 0.0f;

    Devices devices{config.debug, instance, surface};
#ifdef DEBUG
    Debugger debugging{config.debug, instance};
#endif
    Buffers buffers{devices};
    Swapchain swapchain{devices, window, config};
    Images images{config, devices, buffers};
    Renderpass renderpass{devices};
    Descriptor descriptor{devices, buffers};
    Pipeline pipeline{devices};
    Processing processing{devices, camera, config, window};
};
}
