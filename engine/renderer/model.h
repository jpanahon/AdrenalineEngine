/*  
	model.h
	Adrenaline Engine
	
	Insert Description
	Copyright © 2021 Stole Your Shoes. All rights reserved.

*/

#pragma once
#include "global.h"

struct Vertex;

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

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
            (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}