/*
    renderer.h
    Adrenaline Engine
     
    This has the function and class definitions for the renderer.
*/

#pragma once
#include <iostream>
#include <vector>

#include "processing.h"
#include "display.h"
#include "devices.h" 
#include "debugging.h"
#include "gui.h"

namespace Adren {

class Model;

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
    std::vector<Texture> textures;
    
    VkInstance instance;
    Camera camera;
    VmaAllocator allocator;
    
    Display display{instance, camera};
    Devices devices{config.debug, instance, display.surface};
    Debugger debugging{config.debug, instance};
    Swapchain swapchain{devices, display.window, config, allocator};
    Processing processing{swapchain, camera};
    GUI gui{processing, instance};
};
}
