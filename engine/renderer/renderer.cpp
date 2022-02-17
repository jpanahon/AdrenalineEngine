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


void Adren::Renderer::createInstance() {
    if (debug && !devices.checkValidationLayerSupport()) {
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
    if (debug) {
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
    std::cerr << "Initializing program.." << "\n \n" << std::endl;
    createInstance(); std::cerr << "Instance created.." << "\n \n" << std::endl;
    debugging.setupDebugMessenger(); std::cerr << "Debug messenger set up.." << "\n \n" << std::endl;
    display.createSurface(); std::cerr << "Surface created.." << "\n \n" << std::endl;
    devices.pickPhysicalDevice(); std::cerr << "Physical device chosen.." << "\n \n" << std::endl;
    devices.createLogicalDevice(); std::cerr << "Logical device created.." << "\n \n" << std::endl;

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorInfo.physicalDevice = devices.physicalDevice;
    allocatorInfo.device = devices.device;
    allocatorInfo.instance = instance;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    swapchain.createSwapChain();  std::cerr << "Swapchain created.." << "\n \n" << std::endl;
    swapchain.createImageViews(); std::cerr << "Image views created.." << "\n \n" << std::endl;
    swapchain.createRenderPass(); std::cerr << "Render pass created.." << "\n \n" << std::endl;
    swapchain.createDescriptorSetLayout(); std::cerr << "Descriptor set layout created.." << "\n \n" << std::endl;
    swapchain.createGraphicsPipeline(); std::cerr << "Graphics pipeline created.." << "\n \n" << std::endl;
    processing.createCommands(); std::cerr << "Command pool and buffers created.." << "\n \n" << std::endl;
    processing.createSyncObjects(); std::cerr << "Sync objects created.." << "\n \n" << std::endl;
    swapchain.createDepthResources(); std::cerr << "Depth resources created.." << "\n \n" << std::endl;
    swapchain.createFramebuffers(); std::cerr << "Framebuffers created.." << "\n \n" << std::endl;

    for (auto& model : models) {
        textures.push_back(processing.createTextureImage(model.texturePath));
    }

    std::cerr << "Model textures created.." << "\n \n" << std::endl;

    processing.displayModels(); std::cerr << "Models displaying.." << "\n \n" << std::endl;
    processing.createVertexBuffer(); std::cerr << "Vertex buffers created.." << "\n \n" << std::endl;
    processing.createIndexBuffer(); std::cerr << "Index buffers created.." << "\n \n" << std::endl;
    swapchain.createUniformBuffers(); std::cerr << "Uniform buffers created.." << "\n \n" << std::endl;
    swapchain.createDynamicUniformBuffers(); std::cerr << "Dynamic uniform buffers created.." << "\n \n" << std::endl;
    swapchain.createDescriptorPool(); std::cerr << "Descriptor pool created.." << "\n \n" << std::endl;
    swapchain.createDescriptorSets(textures); std::cerr << "Descriptor sets created.." << "\n \n" << std::endl;
}

void Adren::Renderer::mainLoop() {
    while (!glfwWindowShouldClose(display.window)) {
        glfwPollEvents();
        
        if (enableGUI) {
            gui.newImguiFrame();
            gui.startGUI();
        }
        processing.drawFrame();
        processInput();
    }

    vkDeviceWaitIdle(devices.device);
}

void Adren::Renderer::run() { 
    display.initWindow();
    initVulkan();
    if (enableGUI) { gui.initImGui(display.window); }
    mainLoop();
    cleanup();
}

void Adren::Renderer::cleanup() {
    for (auto& tex : textures) {
        vkDestroyImageView(devices.device, tex.textureImageView, nullptr);
        vkDestroyImage(devices.device, tex.texture, nullptr);
        vkFreeMemory(devices.device, tex.textureImageMemory, nullptr);
    }

    swapchain.cleanup();
    processing.cleanup();
    if (enableGUI) { gui.cleanup(); }
    debugging.cleanup();
    vmaDestroyAllocator(allocator);
    devices.cleanup();

    glfwDestroyWindow(display.window);
    vkDestroySurfaceKHR(instance, display.surface, nullptr);
    glfwTerminate();

    vkDestroyInstance(instance, nullptr);
}
void Adren::Renderer::processInput() {
    if (glfwGetKey(display.window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.pos += 0.05f * camera.front;
    }

    if (glfwGetKey(display.window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.pos -= 0.05f * camera.front;
    }

    if (glfwGetKey(display.window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * 0.05f;
    }

    if (glfwGetKey(display.window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * 0.05f;
    }

    if (glfwGetKey(display.window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.pos += 0.05f * camera.up;
    }

    if (glfwGetKey(display.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.pos -= 0.05f * camera.up;
    }

    if (glfwGetKey(display.window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        cleanup();
    }
}