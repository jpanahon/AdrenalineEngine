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
#include <algorithm>

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
        instanceInfo.enabledLayerCount = static_cast<uint32_t>((devices.getDebugLayers()).size());
        instanceInfo.ppEnabledLayerNames = (devices.getDebugLayers()).data();

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
    devices.init(surface); Adren::Tools::log("Devices initialized..");
    swapchain.create(window, surface); Adren::Tools::log("Swapchain created..");
    swapchain.createImageViews(images); Adren::Tools::log("Image views created..");
    images.createDepthResources(swapchain.extent); Adren::Tools::log("Depth resources created..");
    renderpass.create(images.depth, swapchain.imgFormat, instance); Adren::Tools::log("Main render pass created..");
    descriptor.createLayout(models); Adren::Tools::log("Descriptor set layout created..");
    pipeline.create(swapchain, descriptor.layout, renderpass.handle); Adren::Tools::log("Graphics pipeline created..");
    createCommands(); Adren::Tools::log("Command pool and buffers created..");
    createSyncObjects(); Adren::Tools::log("Sync objects created..");
    swapchain.createFramebuffers(images.depth, renderpass.handle); Adren::Tools::log("Main framebuffers created..");
    images.loadTextures(instance, models, textures, commandPool); Adren::Tools::log("Model textures created..");
    buffers.createModelBuffers(models, commandPool); Adren::Tools::log("Index buffers created..");
    camera->create(window, buffers, devices.getAllocator()); Adren::Tools::log("Camera created..");
    buffers.createUniformBuffers(swapchain.images, models); Adren::Tools::log("Dynamic uniform buffer created..");
    buffers.updateDynamicUniformBuffer(models);
    descriptor.createPool(swapchain.images); Adren::Tools::log("Descriptor pool created..");
    descriptor.createSets(textures, swapchain.images, camera->cam); Adren::Tools::log("Descriptor sets created..");

#ifdef DEBUG
        Adren::Tools::label(instance, devices.getDevice(), VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)commandPool, "PRIMARY COMMAND POOL");
        Adren::Tools::label(instance, devices.getDevice(), VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderpass.handle, "MAIN RENDER PASS");
#endif
}

void Adren::Renderer::createCommands() {
    QueueFamilyIndices queueFamilyIndices = Adren::Tools::findQueueFamilies(devices.getGPU(), surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    for (int i = 0; i < maxFramesInFlight; i++) {
        Adren::Tools::vibeCheck("COMMAND POOL", vkCreateCommandPool(devices.getDevice(), &poolInfo, nullptr, &frames[i].commandPool));

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = frames[i].commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        Adren::Tools::vibeCheck("ALLOCATE COMMAND BUFFERS", vkAllocateCommandBuffers(devices.getDevice(), &allocInfo, &frames[i].commandBuffer));

#ifdef DEBUG
        Adren::Tools::label(instance, devices.getDevice(), VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)frames[i].commandPool, "FRAME COMMAND POOL");
        Adren::Tools::label(instance, devices.getDevice(), VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)frames[i].commandBuffer, "FRAME COMMAND BUFFER");
#endif
    }

    VkCommandPoolCreateInfo primaryPoolInfo{};
    primaryPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    primaryPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    primaryPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    Adren::Tools::vibeCheck("CREATED PRIMARY COMMAND POOL", vkCreateCommandPool(devices.getDevice(), &primaryPoolInfo, nullptr, &commandPool));
}

void Adren::Renderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        if (vkCreateSemaphore(devices.getDevice(), &semaphoreInfo, nullptr, &frames[i].iSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(devices.getDevice(), &semaphoreInfo, nullptr, &frames[i].rSemaphore) != VK_SUCCESS ||
            vkCreateFence(devices.getDevice(), &fenceInfo, nullptr, &frames[i].fence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Adren::Renderer::render(Camera* camera) {
    ImGui::Render();

    currentFrame = (currentFrame + 1) % maxFramesInFlight;
    vkWaitForFences(devices.getDevice(), 1, &frames[currentFrame].fence, VK_TRUE, UINT64_MAX);

    if (objects < models.size()) {
        objects += models.size() - objects;
        reloadScene(models, camera);
    }

    vkResetFences(devices.getDevice(), 1, &frames[currentFrame].fence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(devices.getDevice(), swapchain.handle, UINT64_MAX, frames[currentFrame].iSemaphore, VK_NULL_HANDLE, &imageIndex);
    auto commandBuffer = frames[currentFrame].commandBuffer;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    //vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    vkResetCommandPool(devices.getDevice(), frames[currentFrame].commandPool, 0);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    gui.beginRenderpass(camera, commandBuffer, pipeline.handle, buffers.vertex, buffers.index);
    
    if (models.size() >= 1) {
        Offset offset{0, 0, 0, 0, 0, buffers.dynamicUniform.align};
        for (Model* model : models) {
            for (Model::Node& node : model->nodes) {
                model->drawNode(commandBuffer, pipeline.layout, node, descriptor.sets[imageIndex], offset);
            }
            offset.texture += model->textures.size();
            offset.dynamic += static_cast<uint32_t>(offset.align);
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

    vkQueueSubmit(devices.getGraphicsQ(), 1, &submitInfo, frames[currentFrame].fence);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain.handle };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(devices.getPresentQ(), &presentInfo);
}

void Adren::Renderer::init(GLFWwindow* window, Camera* camera) { 
    initVulkan(window, camera);
    window = window;
    gui.init(camera, window, surface); 
}

void Adren::Renderer::cleanup(Camera* camera) {
    Adren::Tools::log("Cleaning up!");
    vkDeviceWaitIdle(devices.getDevice());
    buffers.cleanup(); Adren::Tools::log("Buffers cleaned up!");
    renderpass.cleanup(); Adren::Tools::log("Render pass cleaned up!");
    swapchain.cleanup(); Adren::Tools::log("Swapchain cleaned up!");
    descriptor.cleanup(); Adren::Tools::log("Descriptor cleaned up!");
    images.cleanup(); Adren::Tools::log("Images cleaned up!");
    gui.cleanup(); Adren::Tools::log("GUI cleaned up!");
    camera->destroy(devices.getAllocator()); Adren::Tools::log("Camera cleaned up!");
    delete camera;
#ifdef DEBUG 
    debugging.cleanup(); Adren::Tools::log("Debugger cleaned up!");
#endif

    for (Model* m : models) {
        for (auto& tex : m->textures) {
            vkDestroyImageView(devices.getDevice(), tex.view, nullptr);
            vmaDestroyImage(devices.getAllocator(), tex.image, tex.memory);
        }
        delete m;
    }
    Adren::Tools::log("Textures cleaned up!");


    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    devices.cleanup(); Adren::Tools::log("Devices cleaned up!");
}

void Adren::Renderer::reloadScene(std::vector<Model*>& models, Camera* camera) {
    /*
        This function would be the basis of model loading, as buffers and descriptors get updated
        when there is a new model. This is experimental and may be causing lots of performance issues

        What this does is re-render the entire screen when new elements are in. 
    */
    
    vkDeviceWaitIdle(devices.getDevice());

    Adren::Tools::log("Reloading the scene..");

    vmaDestroyBuffer(devices.getAllocator(), buffers.index.buffer, buffers.index.memory);
    Adren::Tools::log("Index buffer destroyed..");
    vmaDestroyBuffer(devices.getAllocator(), buffers.vertex.buffer, buffers.vertex.memory);
    Adren::Tools::log("Vertex buffer destroyed..");
    vmaUnmapMemory(devices.getAllocator(), buffers.dynamicUniform.memory);
    Adren::Tools::log("Dynamic uniform buffer destroyed..");

    for (const Model::Texture& texture : textures) {
        vkDestroyImageView(devices.getDevice(), texture.view, nullptr);
        vmaDestroyImage(devices.getAllocator(), texture.image, texture.memory);
    }
    textures.clear(); Adren::Tools::log("Textures destroyed..");

    vkDestroyDescriptorSetLayout(devices.getDevice(), descriptor.layout, nullptr);
    Adren::Tools::log("Descriptor set layout destroyed..");

    vkDestroyDescriptorPool(devices.getDevice(), descriptor.pool, nullptr);
    Adren::Tools::log("Descriptor pool destroyed..");

    images.loadTextures(instance, models, textures, commandPool);
    Adren::Tools::log("Model textures reloaded..");

    buffers.createModelBuffers(models, commandPool);
    Adren::Tools::log("Model buffers reloaded..");

    buffers.createUniformBuffers(swapchain.images, models);
    buffers.updateDynamicUniformBuffer(models);
    Adren::Tools::log("Dynamic uniform buffers reloaded..");


    descriptor.createLayout(models);
    Adren::Tools::log("Descriptor set layout reloaded..");

    descriptor.createPool(swapchain.images);
    Adren::Tools::log("Descriptor pool reloaded..");

    descriptor.createSets(textures, swapchain.images, camera->cam);
    Adren::Tools::log("Descriptor sets reloaded..");
}

void Adren::Renderer::processInput(GLFWwindow* window, Camera* camera) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    camera->setDelta(deltaTime);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->move(Camera::Direction::Forward);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->move(Camera::Direction::Backward);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->move(Camera::Direction::Left);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->move(Camera::Direction::Right);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->move(Camera::Direction::Jump);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camera->move(Camera::Direction::Crouch);
    }

    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        cleanup(camera);
    }
}

void Adren::Renderer::addModel(char* path) {
    std::string newPath = path;
#ifdef _WIN32 
    std::replace(newPath.begin(), newPath.end(), '\\', '/');
    Adren::Tools::log(newPath);
#endif
    Model* model = new Model(newPath);
    models.push_back(model);
    Adren::Tools::log("New Model added.");
}