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
#include "stb/stb_image.h"

namespace Adren {
class Engine {
public:
    void run();
    void cleanup();
private:
    GLFWwindow* window;
    void makeWindow();
    void loop();
    Camera camera{};
    Renderer renderer{};
    Editor editor;
    RPC* rpc;

    uint32_t objects = 1;
};
}
