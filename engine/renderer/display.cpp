/*
    Adrenaline Engine
    display.cpp

    This handles the definitions of display.h
*/

#include "display.h"
#include <iostream>

void Adren::Display::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;
    app->lastX = xpos;
    app->lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->yaw += xoffset;
    app->pitch += yoffset;

    if (app->pitch > 89.0f)
        app->pitch = 89.0f;
    if (app->pitch < -89.0f)
        app->pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    direction.y = sin(glm::radians(app->pitch));
    direction.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    app->camera.front = glm::normalize(direction);
}

void Adren::Display::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Adren::Display::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    std::string appName = "Adrenaline Engine";
    window = glfwCreateWindow(1280, 720, appName.c_str(), nullptr, nullptr);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    windowHeight = mode->height;
    windowWidth = mode->width;

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Adren::Display::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}