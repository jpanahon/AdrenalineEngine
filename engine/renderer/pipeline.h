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
	Pipeline(Devices* devices) : device(devices->getDevice()) {}
	void create(Swapchain& swapchain, VkDescriptorSetLayout& layout, VkRenderPass& renderpass);
	VkPipeline handle = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	void cleanup();
private:
	static std::vector<uint32_t> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);

	VkDevice& device;
};
}