/*
	buffers.h
	Adrenaline Engine

	This file has declarations of the buffers used in the engine.
*/

#pragma once
#include "types.h"
#include "devices.h"
#include "model.h"
#include "tools.h"

namespace Adren {
class Buffers {
public:
	Buffers(VkInstance& instance, Devices& devices) : device(devices.getDevice()), allocator(devices.getAllocator()),
		gpu(devices.getGPU()), graphicsQueue(devices.getGraphicsQ()), instance(instance) {}

	void createModelBuffers(std::vector<Model*>& models, VkCommandPool& commandPool);
	void createUniformBuffers(std::vector<VkImage>& images, std::vector<Model*>& models);
	void updateDynamicUniformBuffer(std::vector<Model*>& models);
	void createBuffer(VmaAllocator& allocator, VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer, VmaMemoryUsage vmaUsage);
	void cleanup();

	Buffer vertex;
	Buffer index;
	Buffer dynamicUniform;
	UboData uboData;
private:
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool);
	VmaAllocator& allocator;
	VkDevice& device;
	VkPhysicalDevice& gpu;
	VkQueue& graphicsQueue;
	VkInstance& instance;
};
}