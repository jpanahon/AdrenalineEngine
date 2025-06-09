/*
	buffers.h
	Adrenaline Engine

	This file has declarations of the buffers used in the engine.
*/

#pragma once
#include "devices.h"

namespace Adren {
class Buffer {
public:
	Buffer(Devices* devices, VkDeviceSize& size, VkDeviceSize align = 0) : devices(devices) {}
	void create(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	void cleanup();

	VkBuffer buffer;
    VmaAllocation memory;
    VkDeviceSize size = 0;
    VkDeviceSize align = 0;
    void* mapped;
	void copyTo(VkBuffer dstBuffer, VkCommandPool& commandPool);
private:
	Devices* devices;
};
}