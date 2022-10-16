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
	Images(Devices& devices, Buffers& buffers) : device(devices.getDevice()), buffers(buffers), 
		gpu(devices.getGPU()), graphicsQueue(devices.getGraphicsQ()), allocator(devices.getAllocator()) {}

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VmaMemoryUsage vmaUsage, Image& image);
	VkImageView createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags);
	void loadTextures(VkInstance& instance, std::vector<Model*>& models, std::vector<Model::Texture>& textures, VkCommandPool& commandPool);
	void createDepthResources(VkExtent2D extent);
	void cleanup();
	Image depth = {};
private:
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkCommandPool& commandPool);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandPool& commandPool);
	VkDevice& device;
	VkPhysicalDevice& gpu;
	VkQueue& graphicsQueue;
	Buffers& buffers;
	VmaAllocator& allocator;
};
}