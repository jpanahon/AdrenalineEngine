/*
    renderer.h
    Adrenaline Engine
     
    This has the function and class definitions for the renderer.
    Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#pragma once
#include <iostream>
#include <vector>

#include "info.h"
#include "pipeline.h"
#include "processing.h"
#include "devices.h" 
#include "debugging.h"
#include "model.h"
#include "display.h"
#include "global.h"

struct RendererVariables;

namespace Adren {
class Renderer {
public:
    Renderer(uint32_t windowHeight, uint32_t windowWidth, std::string appName, std::vector<Model>& models) : windowHeight(windowHeight), windowWidth(windowWidth), appName(appName), models(models) {
    }
    void run();
private:
    void createInstance();
    void initVulkan();
    void mainLoop();
    void cleanup();

    uint32_t windowHeight;
    uint32_t windowWidth;
    std::string appName;
    std::vector<Model>& models;
    
    RendererVariables variables{};
    Display display{variables, windowWidth, windowHeight, appName};
    Pipeline pipeline{variables};
    Debugger debugging{variables};
    Devices devices{variables};
    Processing processing{variables, pipeline, models, display};
};
}
