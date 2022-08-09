/*
    Adrenaline Engine
    camera.cpp

    This handles the definitions of camera.h
*/

#define GLFW_INCLUDE_VULKAN
#include "camera.h"
#include "types.h"
#include <iostream>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>


// This function records the cursor's position on the window.
void Adren::Camera::callback(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));

    // This statement is for checking if it was the first time the mouse has moved.
    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    // This is setting the x and y offset so that the pitch and yaw will be relative to the x and y position
    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;

    app->lastX = xpos;
    app->lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->yaw += xoffset;
    app->pitch += yoffset;

    // This prevents the view from being 360 vertically.
    if (app->pitch > 89.0f)
        app->pitch = 89.0f;
    if (app->pitch < -89.0f)
        app->pitch = -89.0f;

    // This converts the pitch and yaw coordinates into the 3d space by turning it to a vector.
    glm::vec3 direction;
    direction.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    direction.y = sin(glm::radians(app->pitch));
    direction.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));

    if (app->toggled) {
        app->front = glm::normalize(direction);
    }
}

void Adren::Camera::create(Buffers& buffers, VmaAllocator& allocator) {
    CameraObject camera;
    cam.size = sizeof(camera);

    buffers.createBuffer(allocator, cam.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        cam, VMA_MEMORY_USAGE_GPU_ONLY);
    vmaMapMemory(allocator, cam.memory, &cam.mapped);
    memcpy(cam.mapped, &camera, cam.size);
}

void Adren::Camera::update() {
    CameraObject camera{};
    camera.view = glm::lookAt(pos, pos + front, up);

    float screen = (float)width / (float)height;
    uint32_t distance = drawDistance * 1000;
    camera.proj = glm::perspective(glm::radians((float)fov), screen, 0.1f, (float)distance);
    camera.proj[1][1] *= -1;
    memcpy(cam.mapped, &camera, sizeof(camera));
}