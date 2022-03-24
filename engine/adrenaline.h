/*
    adrenaline.h
    Adrenaline Engine

    This file combines everything in the game engine together.
*/

#pragma once
#include "renderer/renderer.h"
#include "discord/discord.h"

namespace Adren {
class Engine {
public:
    Engine(Config& config) : config(config) {}
    
    void run();
private:
    Config& config;
    RPC* rpc;
};
}
