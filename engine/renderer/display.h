/*
	Adrenaline Engine
	display.h 
	
	This handles creating the window.
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "types.h"

namespace Adren {
class Display {
public:
	Display(VkInstance& instance, Camera& camera) : instance(instance), camera(camera) {}

	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void initWindow();
	void createSurface();

	bool framebufferResized = false;
	GLFWwindow* window;
	VkSurfaceKHR surface;

private:
	VkInstance& instance;
	Camera& camera;
};
}