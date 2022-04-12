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
#include <glm/gtc/type_ptr.hpp>

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
    ImGui_ImplVulkan_Init(&guiInfo, base.renderpass);

    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, base.commandPool);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, base.commandPool);

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    base.set = ImGui_ImplVulkan_AddTexture(base.sampler, base.color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    Adren::Tools::log("ImGui has been initialized..");
}

void Adren::GUI::cleanup() {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyCommandPool(device, base.commandPool, nullptr);
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
    
    viewport();
}


void Adren::GUI::cameraInfo(bool* open) {
    ImGui::Begin("Camera Properties", open);
    if (ImGui::BeginTabBar("CameraTabBar")) {
        if (ImGui::BeginTabItem("Information")) {
            ImGui::Text("Front: X: %.3f, Y: %.3f, Z: %.3f \n", camera.front.x, camera.front.y, camera.front.z);
            ImGui::Text("Pos: X: %.3f, Y: %.3f, Z: %.3f \n", camera.pos.x, camera.pos.y, camera.pos.z);
            ImGui::Text("Up: X: %.3f, Y: %.3f, Z: %.3f \n", camera.up.x, camera.up.y, camera.up.z);
            ImGui::Text("Last X: %.f \n", camera.lastX);
            ImGui::Text("Last Y: %.f \n", camera.lastY);
            ImGui::Text("Camera Resolution: %dx%d", camera.width, camera.height);
            ImGui::EndTabItem();
        }

        const char* format = "%.3f";
        if (ImGui::BeginTabItem("Viewport Settings")) {
            ImGui::Text("Camera FOV");
            ImGui::SliderInt("30 to 120", &camera.fov, 30, 120);
            ImGui::Text("Camera Draw Distance (x 1000)");
            ImGui::SliderInt("1 to 100", &camera.drawDistance, 1, 100);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Adjustment")) {
            ImGui::InputFloat3("Camera Front", glm::value_ptr(camera.front));
            ImGui::InputFloat3("Camera Position", glm::value_ptr(camera.pos));
            ImGui::Text("Camera Speed");
            ImGui::SliderFloat("0.001 to 100.0", &camera.speed, 0.001f, 100.0f, format);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

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
    bool yep = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    ImVec2 windowSize = ImVec2(base.width, base.height);
    
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetWindowSize(windowSize);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGui::Begin("Viewport", &yep, flags);
    ImGui::Image((ImTextureID)base.set, ImVec2(base.width, base.height));
    ImGui::End();
    ImGui::PopStyleVar();
}

void Adren::GUI::createRenderPass() {
    images.createImage(base.width, base.height, swapchain.imgFormat, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VMA_MEMORY_USAGE_GPU_ONLY, base.color);
    
    base.color.view = images.createImageView(base.color.image, swapchain.imgFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    images.createImage(base.width, base.height, images.depth.format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VMA_MEMORY_USAGE_GPU_ONLY, base.depth);

    base.depth.view = images.createImageView(base.depth.image, images.depth.format, VK_IMAGE_ASPECT_DEPTH_BIT);
   
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    Adren::Tools::vibeCheck("IMGUI SAMPLER", vkCreateSampler(device, &samplerInfo, nullptr, &base.sampler));

    std::array<VkAttachmentDescription, 2> attachments{};
    attachments[0].format = swapchain.imgFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    attachments[1].format = images.depth.format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorReference;
    subpass.pDepthStencilAttachment = &depthReference;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();


    Adren::Tools::vibeCheck("RENDER PASS", vkCreateRenderPass(device, &renderPassInfo, nullptr, &base.renderpass));
}

void Adren::GUI::createFramebuffers() {
    VkImageView attachments[2];
    attachments[0] = base.color.view;
    attachments[1] = base.depth.view;

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = base.renderpass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = base.width;
    framebufferInfo.height = base.height;
    framebufferInfo.layers = 1;

    Adren::Tools::vibeCheck("IMGUI FRAME BUFFER", vkCreateFramebuffer(device, &framebufferInfo, nullptr, &base.framebuffer));
}

void Adren::GUI::createCommands() {
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFam.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    Adren::Tools::vibeCheck("CREATED IMGUI COMMAND POOL", vkCreateCommandPool(device, &commandPoolInfo, nullptr, &base.commandPool));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = base.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    Adren::Tools::vibeCheck("IMGUI COMMAND BUFFER", vkAllocateCommandBuffers(device, &allocInfo, &base.commandBuffer));
    
}

void Adren::GUI::recordGUI(VkCommandBuffer& buffer, Buffers& buffers, VkPipeline& pipeline, VkPipelineLayout& layout, uint32_t& index) {
    VkRenderPassBeginInfo renderpassInfo{};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassInfo.renderPass = base.renderpass;
    renderpassInfo.framebuffer = base.framebuffer;
    renderpassInfo.renderArea.offset = { 0, 0 };
    renderpassInfo.renderArea.extent.width = base.width;
    renderpassInfo.renderArea.extent.height = base.height;

    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.119f, 0.181f, 0.254f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderpassInfo.clearValueCount = 2;
    renderpassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(buffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{};
    viewport.width = (float)base.width;
    viewport.height = (float)base.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    
    VkRect2D scissor{};
    scissor.extent.width = base.width;
    scissor.extent.height = base.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    vkCmdSetViewport(buffer, 0, 1, &viewport);
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = { buffers.vertex.buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(buffer, buffers.index.buffer, 0, VK_INDEX_TYPE_UINT32);
    Offset offset{};
    offset.firstIndex = 0;
    offset.vertexOffset = 0;
    offset.textureOffset = 0;
    offset.dynamicOffset = 0;
    offset.modelOffset = 0;
    offset.dynamicAlignment = buffers.dynamicAlignment;
    for (int m = 0; m < config.models.size(); m++) {
        offset.modelOffset += config.models[m].offset();
        for (size_t n = 0; n < config.models[m].nodes.size(); n++) {
            Model::Node node = config.models[m].nodes[n];
            config.models[m].drawNode(buffer, layout, node, descriptor.sets[index], offset, buffers.dynamicAlignment);
            //offset.dynamicOffset += offset.modelOffset * static_cast<uint32_t>(buffers.dynamicAlignment);
        }
        offset.textureOffset += config.models[m].textures.size();
        offset.dynamicOffset += static_cast<uint32_t>(buffers.dynamicAlignment);
    }
    

    vkCmdEndRenderPass(buffer);
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

