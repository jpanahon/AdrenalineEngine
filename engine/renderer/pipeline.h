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
	void create(std::pair<std::string, std::string>& shaderFiles, 
		Swapchain& swapchain, VkDescriptorSetLayout& layout, VkRenderPass& renderpass);

	VkPipeline handle = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	void cleanup();
private:
	VkDevice& device;
	static std::vector<uint32_t> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);

	VkAttachmentDescription colorAttachment(VkFormat& format);
	VkAttachmentDescription depthAttachment(VkFormat& format);
	VkSubpassDependency dependency();
	VkPipelineShaderStageCreateInfo vertShaderStageInfo();
	VkPipelineShaderStageCreateInfo fragShaderStageInfo();
	VkPipelineInputAssemblyStateCreateInfo inputAssembly();
	VkViewport viewport();
	VkRect2D scissor();
	VkPipelineViewportStateCreateInfo viewportState();
	VkPipelineRasterizationStateCreateInfo rasterizer();
	VkPipelineMultisampleStateCreateInfo multisampling();
	VkPipelineDepthStencilStateCreateInfo depthStencil();
	VkPipelineColorBlendAttachmentState colorBlendAttachment();
	VkPipelineColorBlendStateCreateInfo colorBlending();
	VkSamplerCreateInfo samplerInfo();
};
}