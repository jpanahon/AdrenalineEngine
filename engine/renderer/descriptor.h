/*
	descriptor.h
	Adrenaline Engine

	This file handles all the things related to descriptor sets.
*/

#pragma once
#include "buffers.h"

namespace Adren {
class Descriptor {
public:
	Descriptor(Devices& devices, Buffers& buffers) : devices(devices), buffers(buffers) {}

	void createLayout(std::vector<Model>& models);
	void createPool(std::vector<VkImage>& images);
	void createSets(std::vector<Model::Texture>& textures, std::vector<VkImage>& images);

	std::vector<VkDescriptorSet> sets;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	VkDescriptorPool pool = VK_NULL_HANDLE;
private:
	void fillWrites(std::array<VkWriteDescriptorSet, 4>& write, int index, VkDescriptorSet& dSet, int binding, VkDescriptorType type, size_t& count);
	Buffers& buffers;
	Devices& devices;
	VkDevice& device = devices.device;
	VkSampler sampler;
};
}