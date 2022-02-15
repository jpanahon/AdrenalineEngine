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
#include "devices.h" 
#include "debugging.h"
#include "gui.h"

namespace Adren {

class Model;

class Renderer {
public:
    Renderer(std::vector<Model>& models, bool& debug, bool& enableGUI) : models(models), debug(debug), enableGUI(enableGUI) { 
        run(); 
    }

    void run();
private:
    void createInstance();
    void initVulkan();
    void processInput();
    void mainLoop();
    void cleanup();

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void initWindow();
    void createSurface();

    bool framebufferResized = false;
    GLFWwindow* window;
    VkSurfaceKHR surface;

    std::vector<Model>& models;
    std::vector<Texture> textures;
    bool& enableGUI;
    int modelCount = models.size();

    bool firstMouse = true;
    int windowWidth = 0;
    int windowHeight = 0;
    float lastX = windowWidth / 2.0;
    float lastY = windowHeight / 2.0;
    float yaw = -90.0f;
    float pitch = 0.0f;

    VkInstance instance;
    bool& debug;
    Camera camera;
    VmaAllocator allocator;
    
    Devices devices{debug, instance, surface};
    Debugger debugging{debug, instance};
    Swapchain swapchain{devices.device, devices.physicalDevice, surface, window, modelCount, allocator};
    Processing processing{swapchain, devices.device, devices.physicalDevice, models, devices.graphicsQueue, 
        devices.presentQueue, surface, window, camera, enableGUI, allocator};
    GUI gui{devices.device, instance, devices.physicalDevice, devices.graphicsQueue, swapchain.swapChainImages, 
        swapchain.renderPass, processing.commandPool, surface, window};
};
}
