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
    Renderer(std::vector<Model>& models, bool& debug, bool& enableGUI) : models(models), debug(debug), enableGUI(enableGUI) {}

    void run();
private:
    void createInstance();
    void initVulkan();
    void processInput();
    void mainLoop();
    void cleanup();

    std::vector<Model>& models;
    std::vector<Texture> textures;
    bool& enableGUI;
    int modelCount = models.size();

    VkInstance instance;
    bool& debug;
    Camera camera;
    VmaAllocator allocator;
    
    Display display{instance, camera};
    Devices devices{debug, instance, display.surface};
    Debugger debugging{debug, instance};
    Swapchain swapchain{devices.device, devices.physicalDevice, display.surface, display.window, modelCount, allocator};
    Processing processing{swapchain, devices.device, devices.physicalDevice, models, devices.graphicsQueue, devices.presentQueue, display.surface, display.window, camera, enableGUI, allocator};
    GUI gui{devices.device, instance, devices.physicalDevice, devices.graphicsQueue, swapchain.swapChainImages, swapchain.renderPass, processing.commandPool, display.surface, display.window};
};
}
