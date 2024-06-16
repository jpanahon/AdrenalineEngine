/*
    renderer.cpp
    Adrenaline Engine

    This initializes the Vulkan API.
*/

#define GLFW_INCLUDE_VULKAN
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#ifdef ADREN_DEBUG
    #define VMA_DEBUG_LOG(format, ...) do { \
           printf(format, __VA_ARGS__); \
           printf("\n"); \
       } while(false)
#endif

#include "renderer.h"
#include "info.h"
#include "tools.h"
#include <chrono>
#include <algorithm>

void Adren::Renderer::createInstance() {
#ifdef ADREN_DEBUG
    if (!Adren::Renderer::devices->checkDebugSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }
#endif

    VkApplicationInfo appInfo = Adren::Info::appInfo();
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
    instanceInfo.pApplicationInfo = &appInfo;
    
    auto extensions = devices->getRequiredExtensions();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    
#ifdef ADREN_DEBUG
    instanceInfo.enabledLayerCount = static_cast<uint32_t>((devices->getDebugLayers()).size());
    instanceInfo.ppEnabledLayerNames = (devices->getDebugLayers()).data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    fillDebugInfo(debugCreateInfo);
    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#else
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.pNext = nullptr;
#endif
    
#ifdef ADREN_DEBUG
    Adren::Debugger::vibeCheck("CREATE INSTANCE", vkCreateInstance(&instanceInfo, nullptr, &instance));
#endif

    vkCreateInstance(&instanceInfo, nullptr, &instance);
}


#ifdef ADREN_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL Adren::Renderer::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    std::string severityString;
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        severityString = "VERBOSE";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        severityString = "INFO";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        severityString = "WARNING";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        severityString = "ERROR";
        break;
    default:
        severityString = "UNKNOWN";
        break;
    }

    std::string messageTypeString;
    switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        messageTypeString = "GENERAL";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        messageTypeString = "VALIDATION";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        messageTypeString = "PERFORMANCE";
        break;
    default:
        messageTypeString = "UNKNOWN";
        break;
    }

    std::string message = "[" + severityString + "] [" + messageTypeString + "]\n" + pCallbackData->pMessage + "\n\n";
    std::cerr << message << std::endl;

    return VK_FALSE;
}
void Adren::Renderer::fillDebugInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult Adren::Renderer::createDebugUtils(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Adren::Renderer::destroyDebugUtils(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) func(instance, debugMessenger, pAllocator);
}

void Adren::Renderer::setupDebugger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    fillDebugInfo(createInfo);

    Adren::Debugger::vibeCheck("DEBUG MESSENGER", 
        createDebugUtils(instance, &createInfo, nullptr, &debugMessenger)
    );
}

#endif
void Adren::Renderer::initVulkan(GLFWwindow* window, Camera& camera) {
    Adren::Debugger::log("Initializing program..");

    // This function sets up the Vulkan instance.
    createInstance(); Adren::Debugger::log("Instance created..");

    // This sets up the debugger if debug mode is enabled.
#ifdef ADREN_DEBUG
    setupDebugger(); Adren::Debugger::log("Debug messenger set up..");
#endif

    glfwCreateWindowSurface(instance, window, nullptr, &surface); Adren::Debugger::log("Surface created..");
    devices->init(surface); Adren::Debugger::log("Devices initialized..");
    swapchain.create(window, surface); Adren::Debugger::log("Swapchain created..");
    swapchain.createImageViews(images); Adren::Debugger::log("Image views created..");
    images.createDepthResources(swapchain.extent); Adren::Debugger::log("Depth resources created..");
    renderpass.create(images.depth, swapchain.imgFormat, instance); Adren::Debugger::log("Main render pass created..");
    descriptor.createLayout(models); Adren::Debugger::log("Descriptor set layout created..");
    pipeline.create(swapchain, descriptor.layout, renderpass.handle); Adren::Debugger::log("Graphics pipeline created..");
    createCommands(); Adren::Debugger::log("Command pool and buffers created..");
    createSyncObjects(); Adren::Debugger::log("Sync objects created..");
    swapchain.createFramebuffers(images.depth, renderpass.handle); Adren::Debugger::log("Main framebuffers created..");
    images.loadTextures(instance, models, textures, commandPool); Adren::Debugger::log("Model textures created..");
    buffers.createModelBuffers(models, commandPool); Adren::Debugger::log("Index buffers created..");
    camera.create(window, buffers, devices->getAllocator()); Adren::Debugger::log("Camera created..");
    buffers.createUniformBuffers(swapchain.images, models); Adren::Debugger::log("Dynamic uniform buffer created..");
    buffers.updateDynamicUniformBuffer(models);
    descriptor.createPool(swapchain.images); Adren::Debugger::log("Descriptor pool created..");
    descriptor.createSets(textures, swapchain.images, camera.cam); Adren::Debugger::log("Descriptor sets created..");

#ifdef ADREN_DEBUG
        Adren::Debugger::label(instance, devices->getDevice(), VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)commandPool, "PRIMARY COMMAND POOL");
        Adren::Debugger::label(instance, devices->getDevice(), VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderpass.handle, "MAIN RENDER PASS");
#endif
}

void Adren::Renderer::createCommands() {
    QueueFamilyIndices queueFamilyIndices = Adren::Tools::findQueueFamilies(devices->getGPU(), surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    for (Frame& frame : frames) {
        Adren::Debugger::vibeCheck("FRAME COMMAND POOL", vkCreateCommandPool(devices->getDevice(), &poolInfo, nullptr, &frame.commandPool));
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = frame.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };


#ifdef ADREN_DEBUG
        Adren::Debugger::vibeCheck("ALLOCATE COMMAND BUFFERS", vkAllocateCommandBuffers(devices->getDevice(), &allocInfo, &frame.commandBuffer));
        Adren::Debugger::label(instance, devices->getDevice(), VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)frame.commandPool, "FRAME COMMAND POOL");
        Adren::Debugger::label(instance, devices->getDevice(), VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)frame.commandBuffer, "FRAME COMMAND BUFFER");
#else
        vkAllocateCommandBuffers(devices->getDevice(), &allocInfo, &frame.commandBuffer);
#endif
    }

    VkCommandPoolCreateInfo primaryPoolInfo{};
    primaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    primaryPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    primaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

#ifdef ADREN_DEBUG
    Adren::Debugger::vibeCheck("CREATED PRIMARY COMMAND POOL", vkCreateCommandPool(devices->getDevice(), &primaryPoolInfo, nullptr, &commandPool));
#else
    vkCreateCommandPool(devices->getDevice(), &primaryPoolInfo, nullptr, &commandPool);
#endif
}

void Adren::Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (Frame& frame : frames) {
        if (vkCreateSemaphore(devices->getDevice(), &semaphoreInfo, nullptr, &frame.iSemaphore) != VK_SUCCESS || vkCreateSemaphore(devices->getDevice(), &semaphoreInfo, nullptr, &frame.rSemaphore) != VK_SUCCESS || vkCreateFence(devices->getDevice(), &fenceInfo, nullptr, &frame.fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}

void Adren::Renderer::render(Camera& camera) {
    if (objects < models.size()) {
        objects += models.size() - objects;
        reloadScene(models, camera);
    }

    ImGui::Render();

    currentFrame = (currentFrame + 1) % maxFramesInFlight;
    vkWaitForFences(devices->getDevice(), 1, &frames[currentFrame].fence, VK_TRUE, UINT64_MAX);

    vkResetFences(devices->getDevice(), 1, &frames[currentFrame].fence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(devices->getDevice(), swapchain.handle, UINT64_MAX, frames[currentFrame].iSemaphore, VK_NULL_HANDLE, &imageIndex);
    auto commandBuffer = frames[currentFrame].commandBuffer;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    vkResetCommandPool(devices->getDevice(), frames[currentFrame].commandPool, 0);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    gui.beginRenderpass(camera, commandBuffer, pipeline.handle, buffers.vertex, buffers.index);
    
    
    if (models.size() >= 1) {
        Offset offset{ 0, 0, 0, 0, 0, buffers.dynamicUniform.align };
        for (Model* model : models) {
            for (auto& scene : model->gltfModel.scenes) {
                for (auto& node : scene.nodeIndices) {
                    model->drawNode(node, commandBuffer, pipeline.layout, descriptor.sets[imageIndex], offset);
                }
                offset.texture += model->textures.size();
                offset.dynamic *= static_cast<uint32_t>(offset.align);
            }
            
        }
    }
    
    vkCmdEndRenderPass(commandBuffer);

    renderpass.begin(commandBuffer, imageIndex, swapchain.framebuffers, swapchain.extent);

    gui.draw(commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { frames[currentFrame].iSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    std::array<VkCommandBuffer, 1> commandBuffers = { commandBuffer };
    submitInfo.commandBufferCount = commandBuffers.size();

    submitInfo.pCommandBuffers = commandBuffers.data();

    VkSemaphore signalSemaphores[] = { frames[currentFrame].rSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(devices->getGraphicsQ(), 1, &submitInfo, frames[currentFrame].fence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain.handle };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(devices->getPresentQ(), &presentInfo);
}

void Adren::Renderer::init(GLFWwindow* window, Camera& camera) { 
    initVulkan(window, camera);
    window = window;
    gui.init(camera, window, surface); 
}

void Adren::Renderer::cleanup(Camera& camera) {
#ifdef ADREN_DEBUG
    Adren::Debugger::log("Cleaning up Renderer!");
#endif
    vkDeviceWaitIdle(devices->getDevice());

    vkDestroyCommandPool(devices->getDevice(), commandPool, nullptr);

    for (Frame& frame : frames) {
        vkDestroyCommandPool(devices->getDevice(), frame.commandPool, nullptr);
        vkDestroySemaphore(devices->getDevice(), frame.rSemaphore, nullptr);
        vkDestroySemaphore(devices->getDevice(), frame.iSemaphore, nullptr);
        vkDestroyFence(devices->getDevice(), frame.fence, nullptr);
    }

    Adren::Debugger::log("Rendering objects cleaned up!");
    camera.destroy(devices->getAllocator()); Adren::Debugger::log("Camera cleaned up!");
    buffers.cleanup(); Adren::Debugger::log("Buffers cleaned up!");
    renderpass.cleanup(); Adren::Debugger::log("Render pass cleaned up!");
    swapchain.cleanup(); Adren::Debugger::log("Swapchain cleaned up!");
    pipeline.cleanup(); Adren::Debugger::log("Pipeline cleaned up!");
    descriptor.cleanup(); Adren::Debugger::log("Descriptor cleaned up!");
    images.cleanup(); Adren::Debugger::log("Images cleaned up!");
    
    for (Model* m : models) {
        for (auto& tex : m->textures) {
            vkDestroyImageView(devices->getDevice(), tex.view, nullptr);
            vmaDestroyImage(devices->getAllocator(), tex.image, tex.memory);
        }
        delete m;
    }
    
    for (Image& t : textures) {
        vkDestroyImageView(devices->getDevice(), t.view, nullptr);
        vmaDestroyImage(devices->getAllocator(), t.image, t.memory);
    }

    Adren::Debugger::log("Textures cleaned up!");

    vkDestroySurfaceKHR(instance, surface, nullptr); Adren::Debugger::log("Surface cleaned up!");
    gui.cleanup(); Adren::Debugger::log("GUI cleaned up!");
    devices->cleanup(); Adren::Debugger::log("Devices cleaned up!");

#ifdef ADREN_DEBUG 
    destroyDebugUtils(instance, debugMessenger, nullptr); Adren::Debugger::log("Debugger cleaned up!");
#endif
    vkDestroyInstance(instance, nullptr);
}

void Adren::Renderer::reloadScene(std::vector<Model*>& models, Camera& camera) {
    /*
        This function would be the basis of model loading, as buffers and descriptors get updated
        when there is a new model. This is experimental and may be causing lots of performance issues

        What this does is re-render the entire screen when new elements are in. 
    */
    
    vkDeviceWaitIdle(devices->getDevice());

    Adren::Debugger::log("Reloading the scene..");

    vmaDestroyBuffer(devices->getAllocator(), buffers.index.buffer, buffers.index.memory);
    Adren::Debugger::log("Index buffer destroyed..");
    vmaDestroyBuffer(devices->getAllocator(), buffers.vertex.buffer, buffers.vertex.memory);
    Adren::Debugger::log("Vertex buffer destroyed..");
    vmaUnmapMemory(devices->getAllocator(), buffers.dynamicUniform.memory);
    Adren::Debugger::log("Dynamic uniform buffer destroyed..");

    for (const Model::Texture& texture : textures) {
        vkDestroyImageView(devices->getDevice(), texture.view, nullptr);
        vmaDestroyImage(devices->getAllocator(), texture.image, texture.memory);
    }

    textures.clear(); Adren::Debugger::log("Textures destroyed..");

    vkDestroyDescriptorSetLayout(devices->getDevice(), descriptor.layout, nullptr);
    Adren::Debugger::log("Descriptor set layout destroyed..");

    vkDestroyDescriptorPool(devices->getDevice(), descriptor.pool, nullptr);
    Adren::Debugger::log("Descriptor pool destroyed..");

    images.loadTextures(instance, models, textures, commandPool);
    Adren::Debugger::log("Model textures reloaded..");

    buffers.createModelBuffers(models, commandPool);
    Adren::Debugger::log("Model buffers reloaded..");

    buffers.createUniformBuffers(swapchain.images, models);
    buffers.updateDynamicUniformBuffer(models);
    Adren::Debugger::log("Dynamic uniform buffers reloaded..");


    descriptor.createLayout(models);
    Adren::Debugger::log("Descriptor set layout reloaded..");

    descriptor.createPool(swapchain.images);
    Adren::Debugger::log("Descriptor pool reloaded..");

    descriptor.createSets(textures, swapchain.images, camera.cam);
    Adren::Debugger::log("Descriptor sets reloaded..");
}

void Adren::Renderer::processInput(GLFWwindow* window, Camera& camera) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    camera.setDelta(deltaTime);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.move(Camera::Direction::Forward);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move(Camera::Direction::Backward);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move(Camera::Direction::Left);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move(Camera::Direction::Right);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.move(Camera::Direction::Jump);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camera.move(Camera::Direction::Crouch);
    }

    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        cleanup(camera);
    }
}

void Adren::Renderer::addModel(char* path) {
    std::string newPath = path;

#ifdef _WIN32 
    std::replace(newPath.begin(), newPath.end(), '\\', '/');
    Adren::Debugger::log(newPath);
#endif

    Model* model = new Model(newPath);
    models.push_back(model);
    Adren::Debugger::log("New Model added.");
}