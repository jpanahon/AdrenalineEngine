/*
	pipeline.h
	Adrenaline Engine

	This defines everything related to the graphics pipeline
*/
#pragma once
#include "devices.h"
#include "swapchain.h"

namespace Adren {
class Pipeline {
public:
	Pipeline(Devices& devices) : devices(devices) {}
	void create(Swapchain& swapchain, VkDescriptorSetLayout& layout);
	VkPipeline handle;
	VkPipelineLayout layout = VK_NULL_HANDLE;
private:
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device);

	Devices& devices;
	VkDevice device = devices.device;
};
}