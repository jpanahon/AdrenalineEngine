/*
	textures.h
	Adrenaline Engine

	This handles all the textures for a model.
*/

#pragma once
#GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

class Texture {
public:
	uint32_t width, height = 0;
	VkImageView view = VK_NULL_HANDLE);
	VkImage image = VK_NULL_HANDLE
};