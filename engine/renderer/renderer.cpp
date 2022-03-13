/*
    renderer.cpp
    Adrenaline Engine

    This initializes the Vulkan API.
*/

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include "renderer.h"
#include "info.h"
#include "tools.h"
#include <chrono>


void Adren::Renderer::createInstance() {
    if (config.debug && !devices.checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }
    
    VkApplicationInfo appInfo = Adren::Info::appInfo();
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
    instanceInfo.pApplicationInfo = &appInfo;
    
    auto extensions = devices.getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (config.debug) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(devices.validationLayers.size());
        instanceInfo.ppEnabledLayerNames = devices.validationLayers.data();

        debugging.populateDebugMessengerCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        instanceInfo.enabledLayerCount = 0;
        
        instanceInfo.pNext = nullptr;
    }
    
    Adren::Tools::vibeCheck("CREATE INSTANCE", vkCreateInstance(&instanceInfo, nullptr, &instance));
}

void Adren::Renderer::initVulkan() {
    Adren::Tools::log("Initializing program..");
    createInstance(); Adren::Tools::log("Instance created..");
    debugging.setupDebugMessenger(); Adren::Tools::log("Debug messenger set up..");
    display.createSurface(); Adren::Tools::log("Surface created..");
    devices.pickPhysicalDevice(); Adren::Tools::log("Physical device chosen..");
    devices.createLogicalDevice(); Adren::Tools::log("Logical device created..");
    devices.createAllocator(); Adren::Tools::log("Memory allocator created..");
    swapchain.create(display.surface); Adren::Tools::log("Swapchain created..");
    swapchain.createImageViews(images); Adren::Tools::log("Image views created..");
    images.createDepthResources(swapchain.extent); Adren::Tools::log("Depth resources created..");
    swapchain.createRenderPass(images.depth); Adren::Tools::log("Render pass created..");
    descriptor.createLayout(config.models); Adren::Tools::log("Descriptor set layout created..");
    pipeline.create(swapchain, descriptor.layout); Adren::Tools::log("Graphics pipeline created..");
    processing.createCommands(display.surface); Adren::Tools::log("Command pool and buffers created..");
    processing.createSyncObjects(); Adren::Tools::log("Sync objects created..");
    swapchain.createFramebuffers(images.depth); Adren::Tools::log("Framebuffers created..");
    images.loadTextures(textures, processing.commandPool); Adren::Tools::log("Model textures created..");
    buffers.createModelBuffers(config.models, processing.commandPool); Adren::Tools::log("Index buffers created..");
    buffers.createUniformBuffers(swapchain.images, config.models); Adren::Tools::log("Uniform buffers created..");
    descriptor.createPool(swapchain.images); Adren::Tools::log("Descriptor pool created..");
    descriptor.createSets(textures, swapchain.images); Adren::Tools::log("Descriptor sets created..");
}

void Adren::Renderer::mainLoop() {
    while (!glfwWindowShouldClose(display.window)) {
        glfwPollEvents();
        static auto startTime = std::chrono::high_resolution_clock::now();
        
        if (config.enableGUI) { gui.newImguiFrame(display.window); gui.startGUI(); }
        if (camera.toggled) { buffers.updateUniformBuffer(camera, swapchain.extent); processInput(display.window, camera); }
        processing.render(buffers, pipeline, descriptor, swapchain);
    }

    vkDeviceWaitIdle(devices.device);
}

void Adren::Renderer::run() { 
    display.initWindow();
    initVulkan();
    if (config.enableGUI) { gui.initImGui(display.window, display.surface); }
    mainLoop();
    cleanup();
}

void Adren::Renderer::cleanup() {
    processing.cleanup();
    swapchain.cleanup();
    if (config.enableGUI) { gui.cleanup(); }
    debugging.cleanup();

    for (auto& m : config.models) {
        for (auto& tex : m.textures) {
            vkDestroyImageView(devices.device, tex.view, nullptr);
            vmaDestroyImage(allocator, tex.image, tex.memory);
        }
    }

    devices.cleanup();

    glfwDestroyWindow(display.window);
    vkDestroySurfaceKHR(instance, display.surface, nullptr);
    glfwTerminate();

    vkDestroyInstance(instance, nullptr);
}
void Adren::Renderer::processInput(GLFWwindow* window, Camera& camera) {
    float cameraSpeed = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.pos += cameraSpeed * camera.front;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.pos -= cameraSpeed * camera.front;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.pos += cameraSpeed * camera.up;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camera.pos -= cameraSpeed * camera.up;
    }

    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        cleanup();
    }
}