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
    auto cls = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    // This statement is for checking if it was the first time the mouse has moved.
    if (cls->firstMouse) {
        cls->lastX = xpos;
        cls->lastY = ypos;
        cls->firstMouse = false;
    }

    // This is setting the x and y offset so that the pitch and yaw will be relative to the x and y position
    float xoffset = xpos - cls->lastX;
    float yoffset = cls->lastY - ypos;

    cls->lastX = xpos;
    cls->lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cls->yaw += xoffset;
    cls->pitch += yoffset;

    // This prevents the view from being 360 vertically.
    if (cls->pitch > 89.0f) cls->pitch = 89.0f;
    if (cls->pitch < -89.0f) cls->pitch = -89.0f;

    // This converts the pitch and yaw coordinates into the 3d space by turning it to a vector.
    glm::vec3 direction;
    direction.x = cos(glm::radians(cls->yaw)) * cos(glm::radians(cls->pitch));
    direction.y = sin(glm::radians(cls->pitch));                
    direction.z = sin(glm::radians(cls->yaw)) * cos(glm::radians(cls->pitch));

    if (cls->toggled) {
        cls->front = glm::normalize(direction);
    }
}

void Adren::Camera::create(GLFWwindow* window, Buffers& buffers, VmaAllocator& allocator) {
    CameraObject camera;
    cam.size = sizeof(camera);

    buffers.createBuffer(allocator, cam.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        cam, VMA_MEMORY_USAGE_AUTO);
    vmaMapMemory(allocator, cam.memory, &cam.mapped);
    memcpy(cam.mapped, &camera, cam.size);
}

void Adren::Camera::update() {
    CameraObject camera{};
    camera.view = glm::lookAt(pos, pos + front, up);

    float screen = (float)width / (float)height;
    uint32_t distance = drawDistance * 1000;
    camera.proj = glm::perspective(glm::radians((float)fov), screen, 0.0001f, (float)distance);
    camera.proj[1][1] *= -1;
    memcpy(cam.mapped, &camera, sizeof(camera));
}

void Adren::Camera::move(Direction direction) {
    switch (direction) {
        case Forward: pos += deltaTime * front; break;
        case Backward: pos -= deltaTime * front; break;
        case Left: pos -= glm::normalize(glm::cross(front, up)) * deltaTime; break;
        case Right: pos += glm::normalize(glm::cross(front, up)) * deltaTime; break;
        case Jump: pos += deltaTime * up; break;
        case Crouch: pos -= deltaTime * up; break;
    }
}

void Adren::Camera::destroy(VmaAllocator& allocator) { 
    vmaUnmapMemory(allocator, cam.memory);
    vmaDestroyBuffer(allocator, cam.buffer, cam.memory);
}