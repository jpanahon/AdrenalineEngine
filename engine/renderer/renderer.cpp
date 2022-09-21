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

void Adren::Renderer::initVulkan(GLFWwindow* window, Camera* camera) {
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
    descriptor.createLayout(models); Adren::Tools::log("Descriptor set layout created..");
    pipeline.create(swapchain, descriptor.layout, renderpass.handle); Adren::Tools::log("Graphics pipeline created..");
    processing.createCommands(surface, instance); Adren::Tools::log("Command pool and buffers created..");
    processing.createSyncObjects(); Adren::Tools::log("Sync objects created..");
    swapchain.createFramebuffers(images.depth, renderpass.handle); Adren::Tools::log("Main framebuffers created..");
    images.loadTextures(instance, models, textures, processing.commandPool); Adren::Tools::log("Model textures created..");
    buffers.createModelBuffers(models, processing.commandPool); Adren::Tools::log("Index buffers created..");
    camera->create(window, buffers, devices.allocator); Adren::Tools::log("Camera created..");
    buffers.createUniformBuffers(swapchain.images, models); Adren::Tools::log("Dynamic uniform buffer created..");
    buffers.updateDynamicUniformBuffer(models);
    descriptor.createPool(swapchain.images); Adren::Tools::log("Descriptor pool created..");
    descriptor.createSets(textures, swapchain.images, camera->cam); Adren::Tools::log("Descriptor sets created..");

#ifdef DEBUG
        Adren::Tools::label(instance, devices.device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)processing.commandPool, "PRIMARY COMMAND POOL");
        Adren::Tools::label(instance, devices.device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderpass.handle, "MAIN RENDER PASS");
#endif
}

void Adren::Renderer::process(Camera* camera) {
    processing.render(camera, models, buffers, pipeline, descriptor, swapchain, renderpass, gui);
}

void Adren::Renderer::init(GLFWwindow* window, Camera* camera) { 
    initVulkan(window, camera);
    gui.init(camera, window, surface); 
}

void Adren::Renderer::cleanup(Camera* camera) {
    Adren::Tools::log("Cleaning up!");
    vkDeviceWaitIdle(devices.device);
    processing.cleanup(); Adren::Tools::log("Processing cleaned up!");
    buffers.cleanup(); Adren::Tools::log("Buffers cleaned up!");
    renderpass.cleanup(); Adren::Tools::log("Render pass cleaned up!");
    swapchain.cleanup(); Adren::Tools::log("Swapchain cleaned up!");
    descriptor.cleanup(); Adren::Tools::log("Descriptor cleaned up!");
    images.cleanup(); Adren::Tools::log("Images cleaned up!");
    gui.cleanup(); Adren::Tools::log("GUI cleaned up!");
    camera->destroy(devices.allocator); Adren::Tools::log("Camera cleaned up!");
#ifdef DEBUG 
    debugging.cleanup(); Adren::Tools::log("Debugger cleaned up!");
#endif

    for (auto& m : models) {
        for (auto& tex : m.textures) {
            vkDestroyImageView(devices.device, tex.view, nullptr);
            vmaDestroyImage(devices.allocator, tex.image, tex.memory);
        }
    }
    Adren::Tools::log("Textures cleaned up!");


    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    devices.cleanup(); Adren::Tools::log("Devices cleaned up!");
}

void Adren::Renderer::reloadScene(std::vector<Model>& models, Camera* camera) {
    /*
        This function would be the basis of model loading, as buffers and descriptors get updated
        when there is a new model. This is experimental and may be causing lots of performance issues

        What this does is re-render the entire screen when new elements are in. 
    */
    
    Adren::Tools::log("Reloading the scene..");

    vmaDestroyBuffer(devices.allocator, buffers.index.buffer, buffers.index.memory);
    Adren::Tools::log("Index buffer destroyed..");
    vmaDestroyBuffer(devices.allocator, buffers.vertex.buffer, buffers.vertex.memory);
    Adren::Tools::log("Vertex buffer destroyed..");
    vmaUnmapMemory(devices.allocator, buffers.dynamicUniform.memory);
    Adren::Tools::log("Dynamic uniform buffer destroyed..");

    for (const Model::Texture& texture : textures) {
        vmaDestroyImage(devices.allocator, texture.image, texture.memory);
        vkDestroyImageView(devices.device, texture.view, nullptr);
    }
    textures.clear(); Adren::Tools::log("Textures destroyed..");

    images.loadTextures(instance, models, textures, processing.commandPool);
    Adren::Tools::log("Model textures reloaded..");
    buffers.createModelBuffers(models, processing.commandPool);
    Adren::Tools::log("Model buffers reloaded..");

    buffers.createUniformBuffers(swapchain.images, models);
    Adren::Tools::log("Dynamic uniform buffers reloaded..");

    descriptor.createSets(textures, swapchain.images, camera->cam);
    Adren::Tools::log("Descriptor sets reloaded..");
}

void Adren::Renderer::processInput(GLFWwindow* window, Camera* camera) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    camera->setDelta(deltaTime);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->move(Camera::Move::Forward);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->move(Camera::Move::Backward);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->move(Camera::Move::Left);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->move(Camera::Move::Right);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->move(Camera::Move::Jump);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camera->move(Camera::Move::Crouch);
    }

    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        cleanup(camera);
    }
}

void Adren::Renderer::addModel(char* path) {
    Adren::Tools::log(path);
    Model model{ path };
    models.push_back(model);
}