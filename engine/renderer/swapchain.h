/*
    swapchain.h
    Adrenaline Engine

    This has the declarations of the functions related to the swapchain.
*/

#pragma once
#include <vector>
#include "tools.h"
//#include "devices.h"
#include "images.h"

namespace Adren {
class Devices;

class Swapchain {
public:
    Swapchain(Devices& devices, GLFWwindow* window, Config& config) : devices(devices), window(window), config(config) {}

    void cleanup() {
        for (auto framebuffer : framebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : views) {
            vkDestroyImageView(device, imageView, nullptr);
        }
    }

    void create(VkSurfaceKHR& surface);
    void createFramebuffers(Image& depth, VkRenderPass& renderpass);
    void createImageViews(Images& image);

    VkSwapchainKHR handle = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    VkExtent2D extent;

    std::vector<VkImageView> views;
    std::vector<VkFramebuffer> framebuffers;
    VkFormat imageFormat;
private:
    Devices& devices;
    VkDevice& device = devices.device;
    VkPhysicalDevice& physicalDevice = devices.physicalDevice;
    Config& config;
    GLFWwindow* window;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
};
}