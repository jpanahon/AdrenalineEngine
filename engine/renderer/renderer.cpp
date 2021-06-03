/*
    renderer.cpp
    Adrenaline Engine

    This initializes the Vulkan API.
    Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#include "renderer.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

void Adren::Renderer::createInstance() {
    if (variables.enableValidationLayers && !devices.checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }
    
    VkApplicationInfo appInfo = Adren::Info::appInfo(appName);
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
    instanceInfo.pApplicationInfo = &appInfo;
    
    auto extensions = devices.getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (variables.enableValidationLayers) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames = validationLayers.data();

        debugging.populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        
        instanceInfo.pNext = nullptr;
    }
    
    if (vkCreateInstance(&instanceInfo, nullptr, &variables.instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Adren::Renderer::initVulkan() {
    createInstance();
    debugging.setupDebugMessenger();
    display.createSurface();
    devices.pickPhysicalDevice();
    devices.createLogicalDevice();
    processing.createSwapChain();
    processing.createImageViews();
    processing.createRenderPass();
    pipeline.createDescriptorSetLayout(models.size());
    pipeline.createGraphicsPipeline();
    processing.createCommandPool();
    processing.createDepthResources();
    processing.createFramebuffers(); 

    for (auto& model : models) {
        variables.textures.push_back(processing.createTextureImage(model.texturePath));
    }
    
    processing.createTextureSampler();
    processing.displayModels();
    processing.createVertexBuffer();
    processing.createIndexBuffer();
    processing.createUniformBuffers();
    processing.createDynamicUniformBuffers();
    pipeline.createDescriptorPool();
    pipeline.createDescriptorSets();
    processing.createCommandBuffers();
    processing.createSyncObjects();

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = variables.physicalDevice;
    allocatorInfo.device = variables.device;
    allocatorInfo.instance = variables.instance;
    vmaCreateAllocator(&allocatorInfo, &variables.allocator);
}

void Adren::Renderer::mainLoop() {
    while (!glfwWindowShouldClose(variables.window)) {
        glfwPollEvents();
        
        // display.newImguiFrame();
        // display.startGUI();
        processing.drawFrame();
        display.processInput();
    }


    vkDeviceWaitIdle(variables.device);
}

void Adren::Renderer::cleanup() {
    processing.cleanupSwapChain();
    
    vkDestroySampler(variables.device, variables.textureSampler, nullptr);

    for (auto& tex : variables.textures) {
        vkDestroyImageView(variables.device, tex.textureImageView, nullptr);
        vkDestroyImage(variables.device, tex.texture, nullptr);
        vkFreeMemory(variables.device, tex.textureImageMemory, nullptr);
    }
    
    vkDestroyDescriptorSetLayout(variables.device, variables.descriptorSetLayout, nullptr);
    vkDestroyBuffer(variables.device, variables.indexBuffer, nullptr);
    vkFreeMemory(variables.device, variables.indexBufferMemory, nullptr);
    
    vkDestroyBuffer(variables.device, variables.vertexBuffer, nullptr);
    vkFreeMemory(variables.device, variables.vertexBufferMemory, nullptr);
    
    for (size_t i = 0; i < variables.MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(variables.device, variables.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(variables.device, variables.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(variables.device, variables.inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(variables.device, variables.commandPool, nullptr);
    
    vkDestroyDevice(variables.device, nullptr);
    
    if (variables.enableValidationLayers) {
        debugging.DestroyDebugUtilsMessengerEXT(variables.instance, variables.debugMessenger, nullptr);
    }
    
    // display.shutDownImGui();
    vkDestroySurfaceKHR(variables.instance, variables.surface, nullptr);
    vkDestroyInstance(variables.instance, nullptr);
    
    glfwDestroyWindow(variables.window);
    
    glfwTerminate();

    vmaDestroyAllocator(variables.allocator);
}

void Adren::Renderer::run() { 
    display.initWindow();
    initVulkan();
    // display.initImGui();
    mainLoop();
    cleanup();
}