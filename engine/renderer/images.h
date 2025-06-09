/*
	images.h
	Adrenaline Engine

	This includes the declarations of the functions related to image creation/loading.
*/

#pragma once
#include "devices.h"

namespace Adren {
class Images {
public:
	Images(Devices* devices) : devices(devices) {}
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VmaMemoryUsage vmaUsage, Image& image);
	VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
	void createDepthResources(VkExtent2D extent);
	void cleanup();
private:
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool& commandPool);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool);
	Devices* devices;
};
}