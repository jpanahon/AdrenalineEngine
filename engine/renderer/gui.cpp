/*
    gui.cpp
    Adrenaline Engine

    Definitions for the functions in the gui class that handles the gui you see.
    
*/

#include "gui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "info.h"
#include <glm/gtc/type_ptr.hpp>

void Adren::GUI::init(Camera* camera, GLFWwindow* window, VkSurfaceKHR& surface) {
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
    ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    io.Fonts->AddFontFromFileTTF("../engine/resources/fonts/Montserrat-Regular.ttf", 14);

    ImVec4 uiBlack = ImVec4(0.008f, 0.008f, 0.008f, 1.000f);
    ImVec4 uiPurple = ImVec4(0.431f, 0.235f, 1.000f, 1.000f);
    ImVec4 uiFrame = ImVec4(0.014f, 0.014f, 0.014f, 1.000f);
    ImVec4 uiScroll = ImVec4(0.018f, 0.018f, 0.018f, 1.000f);

    // Changing the default theme.
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

    queueFam = Adren::Tools::findQueueFamilies(gpu, surface);

    createRenderPass(camera);
    createCommands();
    createFramebuffers(camera);

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo guiInfo{};
    guiInfo.Instance = instance;
    guiInfo.PhysicalDevice = gpu;
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
 
#ifdef DEBUG
        Adren::Tools::label(instance, device, VK_OBJECT_TYPE_COMMAND_POOL, (uint64_t)base.commandPool, "IMGUI COMMAND POOL");
        Adren::Tools::label(instance, device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)base.framebuffer, "IMGUI FRAMEBUFFER");
        Adren::Tools::label(instance, device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)base.renderpass, "IMGUI RENDER PASS");
#endif
    Adren::Tools::log("ImGui has been initialized..");
}

void Adren::GUI::cleanup() {
    vkDeviceWaitIdle(device);
    vkDestroyCommandPool(device, base.commandPool, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroySampler(device, base.sampler, nullptr);
    vmaDestroyImage(allocator, base.color.image, base.color.memory);
    vkDestroyImageView(device, base.color.view, nullptr);
    vmaDestroyImage(allocator, base.depth.image, base.depth.memory);
    vkDestroyImageView(device, base.depth.view, nullptr);
    vkDestroyRenderPass(device, base.renderpass, nullptr);
    vkDestroyFramebuffer(device, base.framebuffer, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(); 
}

void Adren::GUI::mouseHandler(GLFWwindow* window, Camera* camera) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        camera->toggled = false;
    } else {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && rightClick == true) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, savedX, savedY);
            rightClick = false;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && rightClick == false) {
        int centerX = camera->getWidth() / 2;
        int centerY = camera->getHeight() / 2;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(window, centerX, centerY);
        savedX = io.MousePos.x;
        savedY = io.MousePos.y;
        rightClick = true;
    }

    if (rightClick) { camera->toggled = false; } else { camera->toggled = true; }

}

void Adren::GUI::newFrame(GLFWwindow* window, Camera* camera) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    mouseHandler(window, camera);
}

void Adren::GUI::createRenderPass(Camera* camera) {
    images.createImage(camera->getWidth(), camera->getHeight(), swapchain.imgFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
        | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VMA_MEMORY_USAGE_AUTO, base.color);
    
    base.color.view = images.createImageView(base.color.image, swapchain.imgFormat, VK_IMAGE_ASPECT_COLOR_BIT);

    images.createImage(camera->getWidth(), camera->getHeight(), images.depth.format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VMA_MEMORY_USAGE_AUTO, base.depth);

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
    
    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    info.pDependencies = dependencies.data();

    Adren::Tools::vibeCheck("RENDER PASS", vkCreateRenderPass(device, &info, nullptr, &base.renderpass));

#ifdef DEBUG 
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_IMAGE, (uint64_t)base.color.image, "GUI COLOR IMAGE");
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_IMAGE, (uint64_t)base.depth.image, "GUI DEPTH IMAGE");
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)base.color.view, "GUI COLOR IMAGE VIEW");
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)base.depth.view, "GUI DEPTH IMAGE VIEW");
#endif
}

void Adren::GUI::createFramebuffers(Camera* camera) {
    VkImageView attachments[2];
    attachments[0] = base.color.view;
    attachments[1] = base.depth.view;

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = base.renderpass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = camera->getWidth();
    framebufferInfo.height = camera->getHeight();
    framebufferInfo.layers = 1;

    Adren::Tools::vibeCheck("IMGUI FRAME BUFFER", vkCreateFramebuffer(device, &framebufferInfo, nullptr, &base.framebuffer));
}

void Adren::GUI::createCommands() {
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = queueFam.graphicsFamily.value();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    Adren::Tools::vibeCheck("CREATED IMGUI COMMAND POOL", vkCreateCommandPool(device, &commandPoolInfo, nullptr, &base.commandPool));
}

// This function recreates images, renderpassand framebuffer used to display the Vulkan scene to the GUI.
// It re-renders the scene in a different display resolution dictated by the viewport function.
void Adren::GUI::resize(Camera* camera) {
    // Destroying the images because we would have to recreate it in a different size.
    vmaDestroyImage(allocator, base.depth.image, base.depth.memory);
    vmaDestroyImage(allocator, base.color.image, base.color.memory);

    // Same with the images
    vkDestroyImageView(device, base.color.view, nullptr);
    vkDestroyImageView(device, base.depth.view, nullptr);
    
    // This destroys the render pass and framebuffer because they are required to render to the viewport
    vkDestroyRenderPass(device, base.renderpass, nullptr);
    vkDestroyFramebuffer(device, base.framebuffer, nullptr);


    // This creates a new renderpass and framebuffer with the new size
    createRenderPass(camera);
    createFramebuffers(camera);
}

// This function sets up the viewport element of the editor that shows what Vulkan is rendering.
void Adren::GUI::viewport(Camera* camera) {

    // Clearing padding and setting the minimum size of the viewport to be 800 x 600.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(800, 600));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;

    ImGui::SetWindowSize(ImVec2(camera->getWidth(), camera->getHeight())); // We are setting the window default window size

    // This positions the viewport in the center of your screen, although you can resize it.
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // We are creating the viewport object itself, the bool is there to make it a non-closable window.
    bool yep = true;
    ImGui::Begin("Viewport", &yep, flags);

    // This will show the current size of the viewport.
    ImVec2 size = ImGui::GetContentRegionAvail();

    // Thank you olkotov for inspiration https://github.com/ocornut/imgui/issues/1287#issuecomment-1093514753
    if (size.x != camera->getWidth() || size.y != camera->getHeight()) {
        if (size.x <= 0 || size.y <= 0) {
            return;
        }

        camera->setWidth(size.x);
        camera->setHeight(size.y);

        vkDeviceWaitIdle(device);
        resize(camera);

        // We are redefining base.set to have the new size.
        base.set = ImGui_ImplVulkan_AddTexture(base.sampler, base.color.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // This is the viewport tabs for when we want to use the space for more than just the viewport.
    if (ImGui::BeginTabBar("ViewportTabBar")) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
        if (ImGui::BeginTabItem("Scene")) {
            ImGui::Image((ImTextureID)base.set, ImVec2(camera->getWidth(), camera->getHeight()));
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Revert back the normal style defined in the editor.
    ImGui::PopStyleVar(3);
    ImGui::End();
}

void Adren::GUI::beginRenderpass(Camera* camera, VkCommandBuffer& buffer, VkPipeline& pipeline, Buffer& vertex, Buffer& index) {
    VkRenderPassBeginInfo renderpassInfo{};
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpassInfo.renderPass = base.renderpass;
    renderpassInfo.framebuffer = base.framebuffer;
    renderpassInfo.renderArea.offset = { 0, 0 };
    renderpassInfo.renderArea.extent.width = camera->getWidth();
    renderpassInfo.renderArea.extent.height = camera->getHeight();

    VkClearValue clearValues[2];
    clearValues[0].color = { 0.119f, 0.181f, 0.254f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderpassInfo.clearValueCount = 2;
    renderpassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(buffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width = (float)camera->getWidth();
    viewport.height = (float)camera->getHeight();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent.width = camera->getWidth();
    scissor.extent.height = camera->getHeight();
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    vkCmdSetViewport(buffer, 0, 1, &viewport);
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    VkBuffer vertexBuffers[] = { vertex.buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(buffer, index.buffer, 0, VK_INDEX_TYPE_UINT32); 
}

void Adren::GUI::draw(VkCommandBuffer& commandBuffer) { 
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer); 
}


