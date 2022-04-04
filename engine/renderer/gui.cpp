/*
    gui.cpp
    Adrenaline Engine

    Definitions for the functions in the gui class that handles the gui you see.
    
*/

#include "gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "info.h"

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

    Adren::Tools::vibeCheck("IMGUI DESCRIPTOR POOL", vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    queueFam = Adren::Tools::findQueueFamilies(physicalDevice, surface);

    guiStyle();
    createRenderPass();
    createCommands();
    createFramebuffers();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo guiInfo{};
    guiInfo.Instance = instance;
    guiInfo.PhysicalDevice = physicalDevice;
    guiInfo.Device = device;
    guiInfo.Queue = graphicsQueue;


    guiInfo.QueueFamily = queueFam.graphicsFamily.value();
    guiInfo.PipelineCache = VK_NULL_HANDLE;
    guiInfo.DescriptorPool = descriptorPool;
    guiInfo.Allocator = VK_NULL_HANDLE;
    guiInfo.MinImageCount = swapchain.imageCount;
    guiInfo.ImageCount = swapchain.imageCount;
    guiInfo.CheckVkResultFn = NULL;
    guiInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&guiInfo, renderPass);

    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    Adren::Tools::log("ImGui has been initialized..");
}

void Adren::GUI::cleanup() {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
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
            ImGui::MenuItem("Camera Properties", " ", &showCameraInfo);
            ImGui::MenuItem("Rendering Information", " ", &showRenderInfo);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    } 

    //viewport();
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
    ImGui::SliderFloat("0.001 to 100.0", &camera.speed, 0.001f, 100.0f, format);
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

    for (int i = 0; i < config.models.size(); i++) {
        Model model = config.models[i];
        std::string name = "Model #" + std::to_string(i + 1);
        if (ImGui::CollapsingHeader(name.c_str())) {
            ImGui::Text("Vertices: %d \n \n", model.vertices.size());
            ImGui::Text("Indices: %d \n \n", model.indices.size());
            ImGui::Text("Textures: %d \n \n", model.textures.size());
            ImGui::Text("Pos: X: %3.f, Y: %3.f, Z: %3.f \n \n", model.position.x, model.position.y, model.position.z);
            ImGui::Text("Scale: %3.f \n \n", model.scale);
            ImGui::Text("Rotation Angle: %3.f \n \n", model.rotationAngle);
            glm::vec3 axis = model.rotationAxis;
            char lAxis;
            if (axis == ADREN_X_AXIS) { lAxis = 'X'; }
            else if (axis == ADREN_Y_AXIS) { lAxis = 'Y'; }
            else { lAxis = 'Z'; }
            ImGui::Text("Rotation Axis: %c \n \n", lAxis);
        }
    }

    ImGui::End();
}

void Adren::GUI::viewport() {
    ImTextureID viewport = ImGui_ImplVulkan_AddTexture(descriptor.sampler, config.models[0].textures[0].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    ImGui::Begin("Viewport");
    ImVec2 panel = ImGui::GetContentRegionAvail();
    ImGui::Image(viewport, ImVec2{ panel.x, panel.y });
    ImGui::End();
}

void Adren::GUI::createRenderPass() {
    VkAttachmentDescription colorAttachment = Adren::Info::colorAttachment(swapchain.imgFormat);

    VkAttachmentReference colorReference{};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;

    VkSubpassDependency dependency = Adren::Info::dependency();

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    Adren::Tools::vibeCheck("RENDER PASS", vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void Adren::GUI::createFramebuffers() {
    framebuffers.resize(swapchain.views.size());

    for (size_t i = 0; i < swapchain.views.size(); i++) {
        VkImageView attachment[1];

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachment;
        framebufferInfo.width = swapchain.extent.width;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.layers = 1;

        attachment[0] = swapchain.views[i];
        Adren::Tools::vibeCheck("SWAPCHAIN FRAME BUFFER", vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]));
    }
}

void Adren::GUI::createCommands() {
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFam.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    Adren::Tools::vibeCheck("CREATED IMGUI COMMAND POOL", vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));
    
    commandBuffers.resize(swapchain.views.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    Adren::Tools::vibeCheck("IMGUI COMMAND BUFFERS", vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));
}

void Adren::GUI::recordGUI(size_t& frame, uint32_t& index) {
    VkCommandBuffer buffer = commandBuffers[frame];
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    info.pInheritanceInfo = nullptr;

    Adren::Tools::vibeCheck("IMGUI BEGIN", vkBeginCommandBuffer(buffer, &info));

    VkRenderPassBeginInfo renderpassInfo{};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassInfo.renderPass = renderPass;
    renderpassInfo.framebuffer = framebuffers[index];
    renderpassInfo.renderArea.offset = { 0, 0 };
    renderpassInfo.renderArea.extent = swapchain.extent;

    VkClearValue color = { 0.119f, 0.181f, 0.254f, 0.0f };

    renderpassInfo.clearValueCount = 1;
    renderpassInfo.pClearValues = &color;

    vkCmdBeginRenderPass(buffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), buffer);

    vkCmdEndRenderPass(buffer);
    vkEndCommandBuffer(buffer);
}

void Adren::GUI::guiStyle() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("../engine/resources/fonts/Montserrat-Regular.ttf", 14);

    ImVec4 uiBlack = ImVec4(0.008f, 0.008f, 0.008f, 1.000f);
    ImVec4 uiPurple = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    ImVec4 uiFrame = ImVec4(0.014f, 0.014f, 0.014f, 1.000f);
    ImVec4 uiScroll = ImVec4(0.018f, 0.018f, 0.018f, 1.000f);

    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowPadding = ImVec2(8.0f, 8.0f);
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
    colors[ImGuiCol_WindowBg] = uiBlack;
    colors[ImGuiCol_TitleBg] = uiPurple;
    colors[ImGuiCol_TitleBgActive] = uiPurple;
    colors[ImGuiCol_TitleBgCollapsed] = uiPurple;
    colors[ImGuiCol_PopupBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.9000f);
    colors[ImGuiCol_Button] = uiPurple;
    colors[ImGuiCol_Header] = uiPurple;
    colors[ImGuiCol_HeaderHovered] = uiPurple;
    colors[ImGuiCol_HeaderActive] = uiPurple;
    colors[ImGuiCol_FrameBg] = uiFrame;
    colors[ImGuiCol_Border] = uiFrame;
    colors[ImGuiCol_ResizeGrip] = uiFrame;
    colors[ImGuiCol_ResizeGripHovered] = uiFrame;
    colors[ImGuiCol_ResizeGripActive] = uiFrame;
    colors[ImGuiCol_MenuBarBg] = uiBlack;
    colors[ImGuiCol_ScrollbarBg] = uiFrame;
    colors[ImGuiCol_ScrollbarGrab] = uiScroll;
    colors[ImGuiCol_ScrollbarGrabActive] = uiScroll;
    colors[ImGuiCol_ScrollbarGrabHovered] = uiScroll;
    colors[ImGuiCol_SliderGrab] = uiPurple;
    colors[ImGuiCol_SliderGrabActive] = uiPurple;
    colors[ImGuiCol_Tab] = uiPurple;
    colors[ImGuiCol_TabActive] = uiPurple;
    colors[ImGuiCol_TabHovered] = uiPurple;
    colors[ImGuiCol_TabUnfocused] = uiPurple;
    colors[ImGuiCol_TabUnfocusedActive] = uiPurple;
}

