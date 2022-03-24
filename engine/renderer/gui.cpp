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
#include <time.h>

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

    ImGui_ImplVulkan_InitInfo guiInfo{};
    guiInfo.Instance = instance;
    guiInfo.PhysicalDevice = physicalDevice;
    guiInfo.Device = device;
    guiInfo.Queue = graphicsQueue;

    QueueFamilyIndices queueFam = Adren::Tools::findQueueFamilies(physicalDevice, surface);

    guiInfo.QueueFamily = queueFam.graphicsFamily.value();
    guiInfo.PipelineCache = VK_NULL_HANDLE;
    guiInfo.DescriptorPool = imguiPool;
    guiInfo.Allocator = VK_NULL_HANDLE;
    guiInfo.MinImageCount = swapchain.images.size();
    guiInfo.ImageCount = swapchain.images.size();
    guiInfo.CheckVkResultFn = NULL;
    guiInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&guiInfo, renderPass);

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
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, savedX, savedY);
            rightClick = false;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && rightClick == false) {
        int centerX = camera.width / 2;
        int centerY = camera.height / 2;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(window, centerX, centerY);
        savedX = io.MousePos.x;
        savedY = io.MousePos.y;
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
    //bool yep = true;
    //ImGui::ShowDemoWindow(&yep);
    
    if (showCameraInfo) { cameraInfo(&showCameraInfo); }
    if (showRenderInfo) { renderInfo(&showRenderInfo); }
   
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Debug")) {
            ImGui::MenuItem("Camera Properties", "ALT + C", &showCameraInfo);
            ImGui::MenuItem("Rendering Information", "ALT + R", &showRenderInfo);
            ImGui::EndMenu();
        }

        //char time[100];
        //auto t = std::time(nullptr);
        //if (std::strftime(time, sizeof(time), "%A, %B %0e, %Y (%I:%M %p)", std::localtime(&t))) {
        //    //ImGui::Text("| %s", time);
        //}

        ImGui::EndMainMenuBar();
    } 
}

void Adren::GUI::cameraInfo(bool* open) {
    ImGui::Begin("Camera Properties", open);
    ImGui::Text("Front: X: %.3f, Y: %.3f, Z: %.3f \n", camera.front.x, camera.front.y, camera.front.z);
    ImGui::Text("Pos: X: %.3f, Y: %.3f, Z: %.3f \n", camera.pos.x, camera.pos.y, camera.pos.z);
    ImGui::Text("Up: X: %.3f, Y: %.3f, Z: %.3f \n", camera.up.x, camera.up.y, camera.up.z);
    ImGui::Text("Last X: %.f \n", camera.lastX);
    ImGui::Text("Last Y: %.f \n", camera.lastY);
    
    ImGui::Separator();
    const char* format = "%.3f";
    ImGui::Text("Camera Speed");
    ImGui::SliderFloat("1.0 to 100.0", &camera.speed, 1.0f, 100.0f, format);
    ImGui::Text("Camera FOV");
    ImGui::SliderInt("30 to 120", &camera.fov, 30, 120);
    ImGui::Text("Camera Draw Distance (x 1000)");
    ImGui::SliderInt("1 to 100", &camera.drawDistance, 1, 100);
    ImGui::End();
}

void Adren::GUI::renderInfo(bool* open) {
    size_t vertices = 0;
    size_t indices = 0;
    size_t textures = 0;
    for (const auto& model : config.models) {
        vertices += model.vertices.size();
        indices += model.indices.size();
        textures += model.textures.size();
    }

    ImGui::Begin("Rendering Information", open);
    ImGui::Text("All Model Vertices: %d \n \n", vertices);
    ImGui::Text("All Model Indices: %d \n \n", indices);
    ImGui::Text("Number of Models: %d \n \n", config.models.size());
    ImGui::Text("Number of Textures: %d", textures);
    ImGui::End();
}

void Adren::GUI::guiStyle() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../engine/resources/fonts/Montserrat-Regular.ttf", 16);

    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
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
