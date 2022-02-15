/*
    adrenaline.h
    Adrenaline Engine

    This file combines everything in the game engine together.
*/

#pragma once
#include "renderer/renderer.h"

namespace Adren {
struct Config {
    std::vector<Model> models;
    bool debug = false;
    bool enableGUI = true;
};

class Engine {
public:
    Engine(Config& config) : config(config) {}
    
    void run();
private:
    Config& config;
};
}
