/*
    renderer.h
    Adrenaline Engine
     
    This has the function and class definitions for the renderer.
*/

#pragma once
#include <iostream>
#include <vector>

#include "processing.h"
// #include "swapchain.h"
#include "window.h"
#include "devices.h" 
#include "debugging.h"
#include "gui.h"

namespace Adren {

class Model;

class Renderer {
public:
    Renderer(std::vector<Model>& models, bool& debug) : models(models), debug(debug) { run(); }
    ~Renderer();
    void run();
private:
    void createInstance();
    void initVulkan();
    void processInput();
    void mainLoop();
    void cleanup();
    GLFWwindow* window;
    std::vector<Model>& models;
    std::vector<Texture> textures;
   
    VkInstance instance;
    bool& debug;
    
    Window display{instance, window};
    Devices devices{debug, instance, display.surface};
    Debugger debugging{debug, instance};
    Swapchain swapchain{devices.device, devices.physicalDevice, display.surface, display.window};
    Processing processing{swapchain, devices.device, devices.physicalDevice, models, devices.graphicsQueue, devices.presentQueue, display.surface, display.window, display.cameraPos, display.cameraFront, display.cameraUp, display.framebufferResized};
    GUI gui{devices.device, instance, devices.physicalDevice, devices.graphicsQueue, swapchain.swapChainImages, swapchain.renderPass, processing.commandPool, display.surface, display.window};
};
}
