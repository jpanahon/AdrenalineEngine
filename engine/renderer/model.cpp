/* 
	model.cpp
	Adrenaline Engine

	This defines the models.
*/

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STBI_MSC_SECURE_CRT

#include "model.h"
#include "tools.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

Model::Model(std::string modelPath) {
    tinygltf::TinyGLTF tinyGLTF;
    std::string error;
    std::string warning;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    bool file = tinyGLTF.LoadASCIIFromFile(&gltf, &error, &warning, modelPath);

    if (!file) { Adren::Tools::log("Unable to load glTF file."); }

    if (!error.empty()) { std::cerr << error; }

    if (!warning.empty()) { std::cerr << warning; }

    if (file) {
        fillImages(gltf);
        fillMaterials(gltf);
        fillTextures(gltf);
        tinygltf::Scene scene = gltf.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = gltf.nodes[scene.nodes[i]];
            fillNode(node, gltf, nullptr, glm::mat4(1.0f));
        }
    }
};

void Model::fillTextures(tinygltf::Model& model) {
    textures.resize(model.textures.size());
    for (size_t t = 0; t < model.textures.size(); t++) {
        textures[t].index = model.textures[t].source;
    }
}

void Model::fillImages(tinygltf::Model& model) {
    images.resize(model.images.size());
    for (size_t i = 0; i < model.images.size(); i++) {
        tinygltf::Image& image = model.images[i];
        unsigned char* imageBuffer = nullptr;
        VkDeviceSize imageBufferSize = 0;
        if (image.component == 3) {
            imageBufferSize = image.width * image.height * 4;
            imageBuffer = new unsigned char[imageBufferSize];
            unsigned char* rgba = imageBuffer;
            unsigned char* rgb = &image.image[0];
            for (size_t i = 0; i < (float)image.width * image.height; ++i) {
                memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                rgba += 4;
                rgb += 3;
            }
        }
        else {
            imageBuffer = &image.image[0];
            imageBufferSize = image.image.size();
        }

        images[i].height = image.height;
        images[i].width = image.width;
        images[i].buffer = imageBuffer;
        images[i].bufferSize = imageBufferSize;
    }
}

void Model::fillMaterials(tinygltf::Model& model) {
    materials.resize(model.materials.size());
    for (size_t m = 0; m < model.materials.size(); m++) {
        tinygltf::Material mat = model.materials[m];

        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            materials[m].baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }

        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            materials[m].baseColorTextureIndex = mat.values["baseColorTexture"].TextureIndex();
        }
    }
}

tinygltf::Accessor Model::getAccessor(const tinygltf::Model& model, const tinygltf::Primitive& prim, std::string attribute) {
    return model.accessors[prim.attributes.find(attribute)->second];
}

void Model::findComponent(const tinygltf::Accessor& accessor, const tinygltf::Buffer& buffer, const tinygltf::BufferView& view) {
    switch (accessor.componentType) {
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
        const uint32_t* index = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
        for (size_t i = 0; i < accessor.count; i++) {
            indices.push_back(index[i]);
        } break;
    }
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
        const uint16_t* index = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
        for (size_t i = 0; i < accessor.count; i++) {
            indices.push_back(index[i]);
        } break;
    }
    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
        const uint8_t* index = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
        for (size_t i = 0; i < accessor.count; i++) {
            indices.push_back(index[i]);
        } break;
    }
    default:
        std::cerr << "Index component type not supported " << accessor.componentType << "\n \n";
        return;
    }
}

glm::mat4 Model::matrix() {
    glm::mat4 matrix(1.0f);
    
    for (Node& node : nodes) {
        matrix = node.matrix;
        Node* parent = node.parent;
        
        while (parent) {
            matrix = parent->matrix * matrix;
            parent = parent->parent;
        }
    }

    return matrix;
}

glm::mat4 Model::getMatrix(const tinygltf::Node& node, glm::mat4x4& base) {
    if (node.matrix.size() == 16) {
        return base * glm::mat4x4(glm::make_mat4x4(node.matrix.data()));
    }
    else {
        auto translation = glm::vec3(0.0f);
        auto rotation = glm::quat();
        auto size = glm::vec3(1.0f);

        if (node.translation.size() == 3) {
            translation = glm::make_vec3(node.translation.data());
        }

        if (node.rotation.size() == 4) {
            rotation = glm::make_quat(node.rotation.data());
        }

        if (node.scale.size() == 3) {
            size = glm::make_vec3(node.scale.data());
        } 

        return base * glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), size);
    }
}
void Model::fillNode(const tinygltf::Node& iNode, const tinygltf::Model& model, Node* parent, glm::mat4& matrix) {
    Node node{};
    node.matrix = getMatrix(iNode, matrix);

    if (iNode.children.size() > 0) {
        for (size_t i = 0; i < iNode.children.size(); i++) {
            fillNode(model.nodes[iNode.children[i]], model, &node, matrix);
        }
    }

    if (iNode.mesh > -1) {
        std::vector<Primitive> primitives;
        const tinygltf::Mesh mesh = model.meshes[iNode.mesh];
        for (size_t p = 0; p < mesh.primitives.size(); p++) {
            const tinygltf::Primitive& prim = mesh.primitives[p];
            const float* modelVert = nullptr;
            const float* modelTex = nullptr;
            uint32_t vertexCount = 0;
            uint32_t indexCount = 0;

            if (prim.attributes.find("POSITION") != prim.attributes.end()) {
                const tinygltf::Accessor& vAccessor = getAccessor(model, prim, "POSITION");
                const tinygltf::BufferView& vBufferView = model.bufferViews[vAccessor.bufferView];
                const tinygltf::Buffer& vBuffer = model.buffers[vBufferView.buffer];
                modelVert = reinterpret_cast<const float*>(&vBuffer.data[vBufferView.byteOffset + vAccessor.byteOffset]);
                vertexCount = vAccessor.count;
            }

            if (prim.attributes.find("TEXCOORD_0") != prim.attributes.end()) {
                const tinygltf::Accessor& tAccessor = getAccessor(model, prim, "TEXCOORD_0");
                const tinygltf::BufferView& tBufferView = model.bufferViews[tAccessor.bufferView];
                const tinygltf::Buffer& tBuffer = model.buffers[tBufferView.buffer];
                modelTex = reinterpret_cast<const float*>(&tBuffer.data[tBufferView.byteOffset + tAccessor.byteOffset]);
            }
               
            for (size_t v = 0; v < vertexCount; v++) {
                Vertex vertex{};
                vertex.pos = glm::vec4(glm::make_vec3(&modelVert[v * 3]), 1.0f);
                vertex.texCoord = modelTex ? glm::make_vec2(&modelTex[v * 2]) : glm::vec3(0.0f);
                vertex.color = glm::vec3(1.0f);
                vertices.push_back(vertex);
            }

            uint32_t firstVertex = static_cast<uint32_t>(vertices.size());
            const tinygltf::Accessor& iAccessor = model.accessors[prim.indices];
            const tinygltf::BufferView& iBufferView = model.bufferViews[iAccessor.bufferView];
            const tinygltf::Buffer& iBuffer = model.buffers[iBufferView.buffer];


            indexCount += static_cast<uint32_t>(iAccessor.count);
            findComponent(iAccessor, iBuffer, iBufferView);

            Primitive primitive{};
            primitive.firstVertex = firstVertex;
            primitive.vertexCount = vertexCount;
            primitive.indexCount = indexCount;
            primitive.firstIndex = static_cast<uint32_t>(indices.size());
            primitive.materialIndex = prim.material;
            node.mesh.primitives.push_back(primitive);
        }
    }

    if (parent) {
        parent->children.push_back(node);
    } else {
        nodes.push_back(node);
    }
}

void Model::count(uint32_t& num, const std::vector<Node>& nodes) {
    for (const auto& node : nodes) {
        count(num, node.children);
        num++;
    }
}

void Model::count(std::vector<glm::mat4>& matrices, const std::vector<Node>& nodes) {
    for (const auto& node : nodes) {
        matrices.push_back(node.matrix);
        count(matrices, node.children);
    }
}

void Model::drawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node& iNode, VkDescriptorSet& set, Offset& offset) {
    if (iNode.mesh.primitives.size() > 0) {
        for (Primitive& prim : iNode.mesh.primitives) {
            if (prim.indexCount > 0) {
                Texture texture = textures[materials[prim.materialIndex].baseColorTextureIndex];
                const auto index = texture.index + offset.texture;
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 1, &offset.dynamic);
                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(index), &index);
                vkCmdDrawIndexed(commandBuffer, prim.indexCount, 1, offset.index, offset.vertex, 0);
                offset.index += prim.indexCount;
                offset.vertex += prim.vertexCount;
            }
        }
    }

    if (iNode.children.size() > 0) {
        for (size_t i = 0; i < iNode.children.size(); i++) {
            offset.dynamic += static_cast<uint32_t>(offset.align);
            drawNode(commandBuffer, pipelineLayout, iNode.children[i], set, offset);
        }
    }
}