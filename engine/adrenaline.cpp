/* 
	adrenaline.cpp
	Adrenaline Engine

	This file defines the entire engine class which connects all the things together.
*/

#include "adrenaline.h"
#define STB_IMAGE_IMPLEMENTATION
#include <cmath>

void Adren::Engine::makeWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // This attempts to replicate 1200 x 700 in different resolutions to make a consistant viewport.
    int32_t width = (int32_t)((double)mode->width * 62.5) / 100;
    int32_t height = (int32_t)round((double)mode->height * 64.81 / 100);
    camera.setWidth(width);
    camera.setHeight(height);

    std::string appName = "Adrenaline Engine";
    window = glfwCreateWindow(mode->width, mode->height, appName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, &camera);

    if (camera.isToggled()) glfwSetCursorPosCallback(window, camera.callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
    GLFWimage images[1];
    images[0].pixels = stbi_load("../engine/resources/textures/Adren2.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}

void Adren::Engine::loop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (camera.isToggled()) {
            renderer.processInput(window, camera);
            camera.update();
        }

        renderer.gui.newFrame(window, camera);
        editor.start(renderer.gui.ctx, camera, renderer);
        renderer.gui.viewport(camera);

        renderer.render(camera);
        
    }

    renderer.wait();

}

void Adren::Engine::run() {
	rpc->initialize();
	rpc->update();

    makeWindow();
    renderer.init(window, camera);
    loop();
    cleanup();
}

void Adren::Engine::cleanup() {
    renderer.cleanup(camera);
    glfwDestroyWindow(window);
    glfwTerminate();
}