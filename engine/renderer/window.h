/* 
	window.h
	Adrenaline Engine

	This file holds the declarations of functions related to the window.
*/

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

namespace Adren {
class Window {
public:
	Window(VkInstance& instance, GLFWwindow* window) : instance(instance), window(window) {}
	void initWindow();
	void createSurface();
	bool framebufferResized = false;
    GLFWwindow* window;
    VkSurfaceKHR surface;
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); 
private:
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	VkInstance& instance;
    bool firstMouse = true;
    int windowWidth;
    int windowHeight;
    float lastX = windowWidth / 2.0;
    float lastY = windowHeight / 2.0;
    float yaw = -90.0f;
    float pitch = 0.0f;
};
}