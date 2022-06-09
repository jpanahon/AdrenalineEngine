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
#include "config.h"

namespace Adren {
class Engine {
public:
    Engine(Config& config) : config(config) {}
    
    void run();
private:
    GLFWwindow* window;
    void makeWindow();
    void loop();
    void cleanup();
    Config& config;
    Renderer renderer{config, window};
    Editor editor;
    Camera& camera = renderer.camera;
    RPC* rpc;
};
}
