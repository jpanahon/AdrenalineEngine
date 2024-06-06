/*
    model.h
    Adrenaline Engine

    This declares the models.
*/

#pragma once
#include "types.h"
#include <fastgltf/glm_element_traits.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Adren {
class Model {
public:
    Model(std::string_view modelPath);

    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        uint32_t vertexCount;
        int32_t materialIndex;
    };

    struct Texture : ::Image {
        int32_t index;
    };

    struct glTFImage {
        unsigned char* buffer = nullptr;
        VkDeviceSize bufferSize = 0;

        int height = 0;
        int width = 0;
    };

    struct Mesh {
        std::vector<Primitive> primitives;
    };

    struct Material {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        uint32_t baseColorTextureIndex = 0;
    };

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture> textures;
    std::vector<glTFImage> images;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<glm::vec2> texcoords;

    fastgltf::Asset gltfModel;

    uint32_t modelSize = 0;
    std::vector<glm::mat4> matrices;

    void drawNode(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout, VkDescriptorSet& set, Offset& offset);
    void countMeshes(uint32_t& num, size_t index);
    void countMatrices(std::vector<glm::mat4>& matrices, size_t index, glm::mat4 matrix);
      
    // Taken from fastgltf's gl_viewer example.
    glm::mat4 getTransformMatrix(const fastgltf::Node& node, glm::mat4x4& base);
    std::vector<Texture> getTextures();
private:
    bool loadImages(fastgltf::Image& image);
    bool loadMaterials(fastgltf::Material& material);
    bool loadTextures(fastgltf::Texture& texture);
    bool loadMesh(fastgltf::Mesh& mesh);
    void drawMesh(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout, VkDescriptorSet& set, Offset& offset);
};
}