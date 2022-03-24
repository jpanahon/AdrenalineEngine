#pragma once
#include "types.h"
#include "devices.h"

namespace Adren {
class Renderpass {
public:
	Renderpass(Devices& devices) : devices(devices) {}

	void create(Image& depth, VkFormat& imageFormat);
	void begin(VkCommandBuffer& commandBuffer, uint32_t& index, std::vector<VkFramebuffer>& framebuffers, VkExtent2D& extent);
	VkRenderPass handle = VK_NULL_HANDLE;
private:
	Devices& devices;
	VkDevice& device = devices.device;
};
}