/*  
	debugging.h
	Adrenaline Engine
	
	Declarations for debugging functions.
*/

#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>

namespace Adren::Debugger {
	inline void log(std::string message) {
		std::cerr << message << std::endl;
	}

	inline void vibeCheck(std::string name, VkResult x) {
		if (x != VK_SUCCESS) {
			std::cerr << "VULKAN ERROR FOR " << name << ": " << x << "\n \n" << std::endl;
			abort();
		}
	}

	inline void label(VkInstance& instance, VkDevice& device, VkObjectType type, uint64_t handle, const char* name) {
		VkDebugUtilsObjectNameInfoEXT info {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, 
			.objectType = type,
			.objectHandle = handle,
			.pObjectName = name
		};
		
		auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
		func(device, &info);
	}
}
