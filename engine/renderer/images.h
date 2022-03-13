/*
	images.h
	Adrenaline Engine

	This includes the declarations of the functions related to image creation/loading.
*/

#pragma once
#include "model.h"
#include "types.h"
#include "buffers.h"

namespace Adren {
class Images {
public:
	Images(Config& config, Devices& devices, Buffers& buffers) : config(config), devices(devices), buffers(buffers) {}

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VmaMemoryUsage vmaUsage, Image& image);
	VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
	void loadTextures(std::vector<Model::Texture>& textures, VkCommandPool& commandPool);
	void createDepthResources(VkExtent2D extent);
	Image depth;
private:
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool& commandPool);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool);
	Devices& devices;
	VkDevice& device = devices.device;
	VkPhysicalDevice& physicalDevice = devices.physicalDevice;
	VkQueue& graphicsQueue = devices.graphicsQueue;
	Buffers& buffers;
	VmaAllocator& allocator = devices.allocator;
	Config& config;
};
}