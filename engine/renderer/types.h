/* 
    types.h
    Adrenaline Engine

    This file holds all the structs.
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>
#include <optional>
#include <array>
#include "vk_mem_alloc.h"

#define ADREN_X_AXIS glm::vec3(1.0f, 0.0f, 0.0f)
#define ADREN_Y_AXIS glm::vec3(0.0f, 1.0f, 0.0f)
#define ADREN_Z_AXIS glm::vec3(1.0f, 0.0f, 1.0f)


struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
            (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 model;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UboDynamicData {
    glm::mat4 *model = nullptr;
};

struct Frame {
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkFence fence;
    VkSemaphore iSemaphore;
    VkSemaphore rSemaphore;
};

struct Camera {
    bool toggled = true;
    bool firstMouse = true;

    glm::vec3 pos = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    int height;
    int width; 

    double lastX = height / 2;
    double lastY = width / 2;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float speed = 1.0f;

    int fov = 90;
    int drawDistance = 10;
};

struct Offset {
    uint32_t firstIndex = 0;
    uint32_t vertexOffset = 0;
    uint32_t textureOffset = 0;
    uint32_t dynamicOffset = 0;
    uint32_t modelOffset = 0;
    VkDeviceSize dynamicAlignment = 0;
};

struct Buffer {
    VkBuffer buffer;
    VmaAllocation memory;
    void* mapped;
};

class Model;

struct Config {
    std::vector<Model> models;
    bool debug = false;
    bool enableGUI = true;
};

struct Image {
    VkImage image;
    VmaAllocation memory;
    VkImageView view;
    VkFormat format;
};
