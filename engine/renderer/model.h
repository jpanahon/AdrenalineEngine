#pragma once
#include "types.h"
#include <tinygltf/tiny_gltf.h>

class Model {
public:
    Model(std::string modelPath);

    struct Node;

    struct Primitive {
        uint32_t firstIndex;
        uint32_t firstVertex;
        uint32_t indexCount;
        uint32_t vertexCount;
        int32_t materialIndex;
    };

    struct Material {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        uint32_t baseColorTextureIndex = 0;
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

    struct Node {
        Node* parent;
        std::vector<Node> children;
        Mesh mesh;
        glm::mat4 matrix;
    };

    struct Matrix {
        glm::mat4 model[4];
    };

    tinygltf::Model gltf;
    std::vector<Texture> textures;
    glm::vec3 position = glm::vec3(0.0f);
    float rotationAngle = 0.0f;
    float scale = 0.0f;
    glm::vec3 rotationAxis = ADREN_Y_AXIS;
    std::vector<Node> nodes;
    std::vector<Material> materials;
    std::vector<glTFImage> images;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::mat4 matrix();
    void count(uint32_t& num, const std::vector<Node>& nodes);
    void count(std::vector<glm::mat4>& matrices, const std::vector<Node>& nodes);
    void drawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node& node, VkDescriptorSet& set, Offset& offset);
private:
    void fillTextures(tinygltf::Model& model);
    void fillMaterials(tinygltf::Model& model);
    void fillImages(tinygltf::Model& model);
    void fillNode(const tinygltf::Node& iNode, const tinygltf::Model& model, Node* parent, glm::mat4& matrix);
    void findComponent(const tinygltf::Accessor& accessor, const tinygltf::Buffer& buffer, 
        const tinygltf::BufferView& view);
    tinygltf::Accessor Model::getAccessor(const tinygltf::Model& model, const tinygltf::Primitive& prim, std::string attribute);

    glm::mat4 getMatrix(const tinygltf::Node& node, glm::mat4x4& base);
};
