/* 
	model.cpp
	Adrenaline Engine

	This defines the models.
*/

#include "model.h"
#include <stdexcept>
#include <iostream>
#include <string>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "types.h"

Adren::Model::Model(std::string modelPath, std::string texturePath, bool player, glm::vec3 position) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    try {
        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str());
    } catch (const std::exception& e) {
        std::cout << warn + err;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            
            vertex.color = {1.0f, 1.0f, 1.0f};
            
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            
            indices.push_back(uniqueVertices[vertex]); 
        }
    }

    this->texturePath = texturePath;
    this->player = player;
    this->position = position;
}

