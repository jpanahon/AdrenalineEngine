/* 
	adrenaline.cpp
	Adrenaline Engine

	This file defines the entire engine class which connects all the things together.
*/

#include "adrenaline.h"
#include "tinygltf/stb_image.h"
#include <cmath>

void Adren::Engine::makeWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // This attempts to replicate 1200 x 700 in different resolutions to make a consistant viewport.
    camera.width = mode->width * 62.5 / 100;
    camera.height = round(mode->height * 64.81 / 100);

    std::string appName = "Adrenaline Engine";
    window = glfwCreateWindow(mode->width, mode->height, appName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
  
    if (camera.toggled) { glfwSetCursorPosCallback(window, camera.callback); }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLFWimage images[1];
    images[0].pixels = stbi_load("../engine/resources/textures/Adren2.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
}

void Adren::Engine::loop() {
    while (!glfwWindowShouldClose(window)) {
        renderer.gui.newFrame(window);
        renderer.gui.viewport();
        editor.start(renderer.gui.ctx);
        renderer.process(window);
        /*
        if (editor.modelPaths.size() > renderer.models.size()) {
            for (int i = renderer.models.size(); i < editor.modelPaths.size(); i++) {
                renderer.addModel(editor.modelPaths[i]);
            }
        }
        
        if (objects < renderer.models.size()) {
            renderer.reloadScene(renderer.models);
            objects += renderer.models.size() - objects;
        }
        */
    }

    renderer.wait();
}

void Adren::Engine::run() {
	rpc->initialize();
	rpc->update();

    makeWindow();
    renderer.init(window);
    loop();
    editor.style(renderer.gui.ctx);
    cleanup();
}

void Adren::Engine::cleanup() {
    renderer.cleanup();
}