/*  
	display.h
	Adrenaline Engine
	
	Handles what is shown on screen.
	Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#pragma once
#include "global.h"

namespace Adren {
static void check_vk_result(VkResult err) {
    if (err == 0) return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0) abort();
}

class Display {
public:
    Display(RendererVariables& renderer, uint32_t WIDTH, uint32_t HEIGHT, std::string appName) : 
        renderer(renderer), WIDTH(WIDTH), HEIGHT(HEIGHT), appName(appName) {
    }

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void initWindow();
    void createSurface();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void processInput();
    void initImGui();
    void shutDownImGui();
    void imguiRender();
    void imGuiDrawData(VkCommandBuffer& commandBuffer);
    void newImguiFrame();
    void startGUI();
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); 
private:
    RendererVariables& renderer;
    uint32_t WIDTH;
    uint32_t HEIGHT;
    std::string appName;
    bool firstMouse = true;
    float lastX = 800.f / 2.0;
    float lastY = 600.f / 2.0;
    float yaw = -90.0f;
    float pitch = 0.0f;
    VkDescriptorPool imguiPool;
    bool show_demo_window = true;
};

// class UI {
// public:
//     UI(RendererVariables& renderer) {
//         this->sampler = renderer.textureSampler;
//         this->vertexBuffer = renderer.vertexBuffer;
//         this->indexBuffer = renderer.indexBuffer;
//         this->vertexCount = renderer.vertexCount;
//         this->indexCount = renderer.indexCount;
//         this->pipelineLayout = renderer.pipelineLayout;
//         this->pipeline = renderer.pipeline;
//         this->descriptorPool = renderer.descriptorPool;
//         this->descriptorSetLayout = renderer.descriptorSetLayout;
//         this->descriptorSet = renderer.descriptorSets;
//         this->device = renderer.device;
//     }
// private:
//     VkSampler sampler;
//     VkBuffer vertexBuffer;
//     VkBuffer indexBuffer;
//     std::vector<int32_t> vertexCount;
//     std::vector<int32_t> indexCount;
//     VkDeviceMemory fontMemory = VK_NULL_HANDLE;
//     VkImage fontImage = VK_NULL_HANDLE;
//     VkImageView fontView = VK_NULL_HANDLE;
//     VkPipelineLayout pipelineLayout;
//     VkPipeline pipeline;
//     VkDescriptorPool descriptorPool;
//     VkDescriptorSetLayout descriptorSetLayout;
//     std::vector<VkDescriptorSet> descriptorSet;
//     VkDevice device;
// }
}
