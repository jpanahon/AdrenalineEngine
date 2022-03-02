/*
    gui.cpp
    Adrenaline Engine

    Definitions for the functions in the gui class that handles the gui you see.
    
*/

#include "gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "tools.h"

void Adren::GUI::initImGui(GLFWwindow* window, VkSurfaceKHR& surface) {
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

    Adren::Tools::vibeCheck("IMGUI DESCRIPTOR POOL", vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    guiStyle();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.Queue = graphicsQueue;

    QueueFamilyIndices queueFam = Adren::Tools::findQueueFamilies(physicalDevice, surface);

    init_info.QueueFamily = queueFam.graphicsFamily.value();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = swapChainImages.size();
    init_info.ImageCount = swapChainImages.size();
    init_info.CheckVkResultFn = NULL;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Adren::GUI::cleanup() {
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(); 
}

void Adren::GUI::mouseHandler(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        camera.toggled = false;
    } else {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && rightClick == true) {
            glfwSetCursorPos(window, camera.lastX, camera.lastY);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            rightClick = false;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && rightClick == false) {
        glfwSetCursorPos(window, camera.lastX, camera.lastY);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        rightClick = true;
    }

    if (rightClick) { camera.toggled = false; } else { camera.toggled = true; }

}

void Adren::GUI::newImguiFrame(GLFWwindow* window) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    mouseHandler(window);
}

void Adren::GUI::startGUI() {
    bool demo = true;
    ImGui::ShowDemoWindow(&demo);
    cameraInfo();
}

void Adren::GUI::cameraInfo() {
    std::string cameraFront = "Camera Front: " + glm::to_string(camera.front) + "\n \n";
    std::string cameraPos = "Camera Position: " + glm::to_string(camera.pos) + "\n \n";
    std::string cameraUp = "Camera Up: " + glm::to_string(camera.up) + "\n \n";
    std::string cameraLastX = "Camera Last X: " + std::to_string(camera.lastX) + "\n \n";
    std::string cameraLastY = "Camera Last Y: " + std::to_string(camera.lastY) + "\n \n";

    ImGui::Begin("Camera Properties", false);
    ImGui::Text(cameraFront.c_str());
    ImGui::Text(cameraPos.c_str());
    ImGui::Text(cameraUp.c_str());
    ImGui::Text(cameraLastX.c_str());
    ImGui::Text(cameraLastY.c_str());
    ImGui::End();
}

void Adren::GUI::guiStyle() {
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.000f, 0.000f, 1.000f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.784f);
    colors[ImGuiCol_Button] = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_Header] = ImVec4(0.31f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.086f, 0.086f, 0.086f, 1.000f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.086f, 0.086f, 0.086f, 1.000f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.086f, 0.086f, 0.086f, 1.000f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.086f, 0.086f, 0.086f, 1.000f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.031f, 0.031f, 0.031f, 1.000f);
}