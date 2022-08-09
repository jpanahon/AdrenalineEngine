/*
	Adrenaline Engine
	camera.h 
	
	This handles the camera functionality.
*/

#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "types.h"
#include "buffers.h"

namespace Adren {
class Camera {
public:
    bool toggled = true;
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.3f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    int32_t height;
    int32_t width;

    Buffer cam;

    float speed = 0.5f;
    int fov = 90;
    int drawDistance = 10;
	static void callback(GLFWwindow* window, double xpos, double ypos);

    // This puts default position of the mouse at the center of the window
    double lastX = width / 2;
    double lastY = height / 2;

    void create(Buffers& buffers, VmaAllocator& allocator);
    void update();
private:
    // This checks if it is the first time the mouse has been used.
    bool firstMouse = true;

    float yaw = -90.0f;
    float pitch = 0.0f;
};
}