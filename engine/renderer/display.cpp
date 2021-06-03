/*
    window.cpp
    Adrenaline Engine

    Definitions for the functions in the Window class that handles the window you see.
    Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#include "display.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

void Adren::Display::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));
    
    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }
    
    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;
    app->lastX = xpos;
    app->lastY = ypos;
    
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    app->yaw += xoffset;
    app->pitch += yoffset;
    
    if (app->pitch > 89.0f)
        app->pitch = 89.0f;
    if(app->pitch < -89.0f)
        app->pitch = -89.0f;
    
    glm::vec3 direction;
    direction.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    direction.y = sin(glm::radians(app->pitch));
    direction.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    app->cameraFront = glm::normalize(direction);
}

void Adren::Display::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    renderer.window = glfwCreateWindow(WIDTH, HEIGHT, appName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(renderer.window, this);
    glfwSetFramebufferSizeCallback(renderer.window, framebufferResizeCallback);
    glfwSetCursorPosCallback(renderer.window, mouse_callback);
    glfwSetInputMode(renderer.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Adren::Display::createSurface() {
    if (glfwCreateWindowSurface(renderer.instance, renderer.window, nullptr, &renderer.surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
   }
}

void Adren::Display::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));
    app->renderer.framebufferResized = true;
}

void Adren::Display::processInput() {
    if (glfwGetKey(renderer.window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += 0.5f * cameraFront;
    }

    if (glfwGetKey(renderer.window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= 0.5f * cameraFront;
    }

    if (glfwGetKey(renderer.window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * 0.5f;
    }

    if (glfwGetKey(renderer.window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * 0.5f;
    }
}

void Adren::Display::initImGui() {
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(renderer.device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(renderer.window, true);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = renderer.instance;
    init_info.PhysicalDevice = renderer.physicalDevice;
    init_info.Device = renderer.device;
    init_info.Queue = renderer.graphicsQueue;
    QueueFamilyIndices queueFam = findQueueFamilies(renderer.physicalDevice, renderer.surface);
    init_info.QueueFamily = queueFam.graphicsFamily.value();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = renderer.swapChainImages.size();
    init_info.CheckVkResultFn = check_vk_result;

    ImGui_ImplVulkan_Init(&init_info, renderer.renderPass);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer.commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderer.device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(renderer.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(renderer.graphicsQueue);
    vkFreeCommandBuffers(renderer.device, renderer.commandPool, 1, &commandBuffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Adren::Display::shutDownImGui() {
    vkDestroyDescriptorPool(renderer.device, imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(); 
}
   
void Adren::Display::imguiRender() {
    ImGui::Render(); 
}

void Adren::Display::imGuiDrawData(VkCommandBuffer& commandBuffer) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void Adren::Display::newImguiFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void Adren::Display::startGUI() {
    ImGui::ShowDemoWindow(&show_demo_window);
}

