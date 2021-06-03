/*
    adrenaline.h
    Adrenaline Engine

    This file combines everything in the game engine together.
    Copyright © 2020 Stole Your Shoes. All rights reserved.
*/

#pragma once
#include "renderer/renderer.h"

namespace Adren {
struct Config {
    uint32_t windowHeight = 1280;
    uint32_t windowWidth = 720;

    std::string appName;
    std::vector<Model> models;

};

class Engine {
public:
    Engine(Config& config) : config(config) {
    }
    
    void setupRenderer();
    void run();
private:
    Config& config;
};
}
