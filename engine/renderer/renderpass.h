#pragma once
#include "types.h"
#include "devices.h"

namespace Adren {
class Renderpass {
public:
	Renderpass(Devices* devices) : device(devices->getDevice()) {}

	void create(Image& depth, VkFormat& imageFormat, VkInstance& instance);
	void begin(VkCommandBuffer& commandBuffer, uint32_t& index, std::vector<VkFramebuffer>& framebuffers, VkExtent2D& extent);
	VkRenderPass handle = VK_NULL_HANDLE;
	void cleanup();
private:
	VkDevice& device;
};
}