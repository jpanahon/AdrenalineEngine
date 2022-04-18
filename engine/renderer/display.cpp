/*
    Adrenaline Engine
    display.cpp

    This handles the definitions of display.h
*/

#define GLFW_INCLUDE_VULKAN
#include "display.h"
#include <iostream>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include "tinygltf/stb_image.h"

/*#ifdef _WIN32
    #include <Windows.h>
    #include <Uxtheme.h>
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#endif*/

void Adren::Display::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));

    if (app->camera.firstMouse) {
        app->camera.lastX = xpos;
        app->camera.lastY = ypos;
        app->camera.firstMouse = false;
    }

    float xoffset = xpos - app->camera.lastX;
    float yoffset = app->camera.lastY - ypos;
    app->camera.lastX = xpos;
    app->camera.lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->camera.yaw += xoffset;
    app->camera.pitch += yoffset;

    if (app->camera.pitch > 89.0f)
        app->camera.pitch = 89.0f;
    if (app->camera.pitch < -89.0f)
        app->camera.pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(app->camera.yaw)) * cos(glm::radians(app->camera.pitch));
    direction.y = sin(glm::radians(app->camera.pitch));
    direction.z = sin(glm::radians(app->camera.yaw)) * cos(glm::radians(app->camera.pitch));
    if (app->camera.toggled) {
        app->camera.front = glm::normalize(direction);
    }
}

void Adren::Display::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Display*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Adren::Display::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    camera.width = 1600;
    camera.height = 1050;
    std::string appName = "Adrenaline Engine";
    window = glfwCreateWindow(mode->height, mode->width, appName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    if (camera.toggled) { glfwSetCursorPosCallback(window, mouse_callback); }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLFWimage images[1];
    images[0].pixels = stbi_load("../engine/resources/textures/Adren2.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

//#ifdef _WIN32
//    HWND win32window = glfwGetWin32Window(window);
//    SetWindowTheme(win32window, L"DarkMode_Explorer", NULL);
//#endif
}

void Adren::Display::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}