/*
    renderer.h
    Adrenaline Engine
     
    This has the function and class definitions for the renderer.
*/

#pragma once
#include <iostream>
#include <vector>

#include "model.h"
//#include "descriptor.h"
//#include "processing.h"
#include "display.h"
//#include "pipeline.h"
// #include "images.h"
// #include "devices.h" 
// #include "buffers.h"
#include "debugging.h"
#include "gui.h"

namespace Adren {
class Renderer {
public:
    Renderer(Config& config) : config(config) {}

    void run();
private:
    void createInstance();
    void initVulkan();
    void processInput(GLFWwindow* window, Camera& camera);
    void mainLoop();
    void cleanup();

    Config& config;
    std::vector<Model::Texture> textures;
    
    VkInstance instance;
    Camera camera;
    VmaAllocator allocator;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Display display{instance, camera};
    Devices devices{config.debug, instance, display.surface};
    Debugger debugging{config.debug, instance};
    Buffers buffers{devices};
    Swapchain swapchain{devices, display.window, config};
    Images images{config, devices, buffers};
    Renderpass renderpass{devices};
    Descriptor descriptor{devices, buffers};
    Pipeline pipeline{devices};
    Processing processing{devices, camera, config, display.window};
    GUI gui{devices, buffers, swapchain, processing, renderpass, instance, camera, config};
};
}
