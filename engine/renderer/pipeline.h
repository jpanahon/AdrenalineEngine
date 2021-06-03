/*
	pipeline.h
	Adrenaline Engine
	
	Header file for pipeline.cpp
	Copyright © 2021 Stole Your Shoes. All rights reserved.
*/

#pragma once
#include "global.h"

namespace Adren {
class Pipeline {
public:
    Pipeline(RendererVariables& renderer) : renderer(renderer) {
    }
    void createGraphicsPipeline();
    void createDescriptorSetLayout(uint32_t models);
    void createDescriptorPool();
    void createDescriptorSets();
private:
    RendererVariables& renderer;
};
}

