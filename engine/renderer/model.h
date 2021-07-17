/*
	model.h
	Adrenaline Engine

	This file holds the class that handles 3D models
 */

#pragma once 
#include "types.h"

namespace Adren {
class Model {
public:
    Model() = default;
    Model(std::string modelPath, std::string texturePath, bool player = false, glm::vec3 position = glm::vec3(0.0f));
        
    std::string texturePath;
    bool player; 
    glm::vec3 position;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
}