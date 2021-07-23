/*
    renderer.cpp
    Adrenaline Engine

    This initializes the Vulkan API.
*/

#include "renderer.h"
#include "info.h"

Adren::Renderer::~Renderer() {
    for (auto& tex : textures) {
        vkDestroyImageView(devices.device, tex.textureImageView, nullptr);
        vkDestroyImage(devices.device, tex.texture, nullptr);
        vkFreeMemory(devices.device, tex.textureImageMemory, nullptr);
    }

    vkDestroySurfaceKHR(instance, display.surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
}

void Adren::Renderer::createInstance() {
    if (debug && !devices.checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }
    
    VkApplicationInfo appInfo = ::appInfo();
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
    instanceInfo.pApplicationInfo = &appInfo;
    
    auto extensions = devices.getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (debug) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(devices.validationLayers.size());
        instanceInfo.ppEnabledLayerNames = devices.validationLayers.data();

        debugging.populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        
        instanceInfo.pNext = nullptr;
    }
    
    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Adren::Renderer::initVulkan() {
    createInstance();
    debugging.setupDebugMessenger();
    display.createSurface();
    devices.pickPhysicalDevice();
    devices.createLogicalDevice();
    swapchain.createSwapChain();
    swapchain.createImageViews();
    swapchain.createRenderPass();
    swapchain.createDescriptorSetLayout(models.size());
    swapchain.createGraphicsPipeline();
    processing.createCommandPool();
    swapchain.createDepthResources();
    swapchain.createFramebuffers();

    for (auto& model : models) {
        textures.push_back(processing.createTextureImage(model.texturePath));
    }

    processing.displayModels();
    processing.createVertexBuffer();
    processing.createIndexBuffer();
    swapchain.createUniformBuffers();
    swapchain.createDynamicUniformBuffers(textures.size());
    swapchain.createDescriptorPool();
    swapchain.createDescriptorSets(textures);
    processing.createCommandBuffers();
    processing.createSyncObjects();
}

void Adren::Renderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        gui.newImguiFrame();
        gui.startGUI();
        processing.drawFrame(textures);
        processInput();
    }

    vkDeviceWaitIdle(devices.device);
}

void Adren::Renderer::run() { 
    display.initWindow();
    initVulkan();
    gui.initImGui();
    mainLoop();
}

void Adren::Renderer::processInput() {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        display.cameraPos += 0.5f * display.cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        display.cameraPos -= 0.5f * display.cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        display.cameraPos -= glm::normalize(glm::cross(display.cameraFront, display.cameraUp)) * 0.5f;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        display.cameraPos += glm::normalize(glm::cross(display.cameraFront, display.cameraUp)) * 0.5f;
    }
}