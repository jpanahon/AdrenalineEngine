/*
	devices.h
	Adrenaline Engine
	
	Definitions for the devices class
	Copyright © 2021 Stole Your Shoes. All rights reserved.

*/

#pragma once
#include "global.h"

namespace Adren {
class Devices {
public:
    Devices(RendererVariables& renderer) : renderer(renderer) {
        
    }
    void pickPhysicalDevice();
    void createLogicalDevice();
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
private:
    RendererVariables& renderer;
};
}
