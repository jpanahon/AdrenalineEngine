/*
    renderer.cpp
    Adrenaline Engine

    This initializes the Vulkan API.
*/
#define GLFW_INCLUDE_VULKAN
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include "renderer.h"
#include "info.h"
#include "tools.h"
#include <chrono>


void Adren::Renderer::createInstance() {
#ifdef DEBUG
    if (!devices.checkDebugSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }
#endif

    VkApplicationInfo appInfo = Adren::Info::appInfo();
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
    instanceInfo.pApplicationInfo = &appInfo;
    
    auto extensions = devices.getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    
#ifdef DEBUG
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(devices.validationLayers.size());
        instanceInfo.ppEnabledLayerNames = devices.validationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        debugging.fillCreateInfo(debugCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#else
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = nullptr;
#endif
    
    Adren::Tools::vibeCheck("CREATE INSTANCE", vkCreateInstance(&instanceInfo, nullptr, &instance));
}

void Adren::Renderer::initVulkan() {
    Adren::Tools::log("Initializing program..");
    createInstance(); Adren::Tools::log("Instance created..");

#ifdef DEBUG
    debugging.setup(); Adren::Tools::log("Debug messenger set up..");
#endif

    glfwCreateWindowSurface(instance, window, nullptr, &surface); Adren::Tools::log("Surface created..");
    devices.pickGPU(); Adren::Tools::log("Graphics Processing Unit chosen..");
    devices.createLogicalDevice(); Adren::Tools::log("Logical device created..");
    devices.createAllocator(); Adren::Tools::log("Memory allocator created..");
    swapchain.create(surface); Adren::Tools::log("Swapchain created..");
    swapchain.createImageViews(images); Adren::Tools::log("Image views created..");
    images.createDepthResources(swapchain.extent); Adren::Tools::log("Depth resources created..");
    renderpass.create(images.depth, swapchain.imgFormat, instance); Adren::Tools::log("Main render pass created..");
    descriptor.createLayout(config.models); Adren::Tools::log("Descriptor set layout created..");
    pipeline.create(swapchain, descriptor.layout, renderpass.handle); Adren::Tools::log("Graphics pipeline created..");
    processing.createCommands(surface, instance); Adren::Tools::log("Command pool and buffers created..");
    processing.createSyncObjects(); Adren::Tools::log("Sync objects created..");
    swapchain.createFramebuffers(images.depth, renderpass.handle); Adren::Tools::log("Main framebuffers created..");
    images.loadTextures(textures, processing.commandPool); Adren::Tools::log("Model textures created..");
    buffers.createModelBuffers(config.models, processing.commandPool); Adren::Tools::log("Index buffers created..");
    buffers.createUniformBuffers(swapchain.images, config.models); Adren::Tools::log("Uniform buffers created..");
    buffers.updateDynamicUniformBuffer(config.models);
    descriptor.createPool(swapchain.images); Adren::Tools::log("Descriptor pool created..");
    descriptor.createSets(textures, swapchain.images); Adren::Tools::log("Descriptor sets created..");

#ifdef DEBUG
        Adren::Tools::label(instance, devices.device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)processing.commandPool, "PRIMARY COMMAND POOL");
        Adren::Tools::label(instance, devices.device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderpass.handle, "MAIN RENDER PASS");
#endif
}

void Adren::Renderer::process(GLFWwindow* window) {
    if (camera.toggled) { buffers.updateUniformBuffer(camera, swapchain.extent); processInput(window, camera); }
    processing.render(buffers, pipeline, descriptor, swapchain, renderpass, gui);
}

void Adren::Renderer::init(GLFWwindow* window) { 
    initVulkan();
    gui.init(window, surface); 
}

void Adren::Renderer::cleanup() {
    processing.cleanup();
    swapchain.cleanup();
    gui.cleanup(); 

#ifdef DEBUG 
    debugging.cleanup();
#endif

    for (auto& m : config.models) {
        for (auto& tex : m.textures) {
            vkDestroyImageView(devices.device, tex.view, nullptr);
            vmaDestroyImage(devices.allocator, tex.image, tex.memory);
        }
    }

    devices.cleanup();

    glfwDestroyWindow(window);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwTerminate();

    vkDestroyInstance(instance, nullptr);
}
void Adren::Renderer::processInput(GLFWwindow* window, Camera& camera) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float speed = camera.speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.pos += speed * camera.front;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.pos -= speed * camera.front;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.pos += speed * camera.up;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camera.pos -= speed * camera.up;
    }

    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        cleanup();
    }
}