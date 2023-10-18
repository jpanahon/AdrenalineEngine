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
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.3f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    enum Direction {
        Forward, Backward, Left, Right, Jump, Crouch
    };

    Buffer cam;
    float speed = 0.5f;
    int fov = 90;
    int drawDistance = 10;
	static void callback(GLFWwindow* window, double xpos, double ypos);

    // This puts default position of the mouse at the center of the window
    double lastX = width / 2;
    double lastY = height / 2;

    void create(GLFWwindow* window, Buffers& buffers, VmaAllocator& allocator);
    void update();
    int32_t getWidth() { return width; }
    int32_t getHeight() { return height; }
    void setWidth(int32_t size) { this->width = size; }
    void setHeight(int32_t size) { this->height = size; }
    bool& isToggled() { return toggled; }
    void disable() { toggled = false; }
    void enable() { toggled = true; }
    void move(Direction direction);
    void setDelta(float delta) { deltaTime = speed * delta; }
    void destroy(VmaAllocator& allocator);
private:
    // This checks if it is the first time the mouse has been used.
    bool toggled = true;
    bool firstMouse = true;
    float deltaTime = 0.0;
    int32_t height = 0;
    int32_t width = 0;
    float yaw = -90.0f;
    float pitch = 0.0f;
};
}