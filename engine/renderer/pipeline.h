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
	Pipeline(Devices& devices) : device(devices.device) {}
	void create(Swapchain& swapchain, VkDescriptorSetLayout& layout, VkRenderPass& renderpass);
	VkPipeline handle = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
private:
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	VkDevice& device;
};
}