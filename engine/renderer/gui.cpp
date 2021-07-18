/*
    window.cpp
    Adrenaline Engine

    Definitions for the functions in the Window class that handles the window you see.
    
*/

#include "gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "tools.h"

void Adren::GUI::initImGui() {
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

    vibeCheck(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool));
    std::cout << "\n \n Passed the vibe check \n \n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.Queue = graphicsQueue;
    QueueFamilyIndices queueFam = findQueueFamilies(physicalDevice, surface);
    init_info.QueueFamily = queueFam.graphicsFamily.value();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = swapChainImages.size();
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device,  commandPool);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Adren::GUI::shutDownImGui() {
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(); 
}
   
void Adren::GUI::newImguiFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void Adren::GUI::startGUI() {
    ImGui::ShowDemoWindow(&demoWindow);
}

