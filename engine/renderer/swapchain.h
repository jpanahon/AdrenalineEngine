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
    Swapchain(Devices* devices) : device(devices->getDevice()), gpu(devices->getGPU()) {}

    void cleanup() {
        for (VkFramebuffer& framebuffer : framebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (VkImageView& imageView : views) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, handle, nullptr);
    }

    void create(GLFWwindow* window, VkSurfaceKHR& surface);
    void createFramebuffers(Image& depth, VkRenderPass& renderpass);
    void createImageViews(Images& image);

    VkSwapchainKHR handle = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    VkExtent2D extent;

    std::vector<VkImageView> views;
    std::vector<VkFramebuffer> framebuffers;
    VkFormat imgFormat;
    uint32_t imageCount;
private:
    VkDevice& device;
    VkPhysicalDevice& gpu;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
};
}