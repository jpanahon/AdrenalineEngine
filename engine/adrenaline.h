/*
    adrenaline.h
    Adrenaline Engine

    This is the main engine file, all the components come together here.
    This file includes the discord rich presence and the Vulkan renderer.
*/

#pragma once
#include "renderer/renderer.h"
#include "discord/discord.h"
#include "editor/editor.h"

namespace Adren {
class Engine {
public:
    void run();
private:
    GLFWwindow* window;
    void makeWindow();
    void loop();
    void cleanup();
    Renderer renderer;
    Camera& camera = renderer.camera;
    Editor editor{camera};
    RPC* rpc;

    uint32_t objects = 1;
};
}
