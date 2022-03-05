/* 
	model.cpp
	Adrenaline Engine

	This defines the models.
*/

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STBI_MSC_SECURE_CRT

#include "types.h"
#include <glm/gtc/type_ptr.hpp>

Model::Model(std::string modelPath) {
    tinygltf::TinyGLTF tinyGLTF;
    std::string error;
    std::string warning;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    bool file = tinyGLTF.LoadASCIIFromFile(&gltf, &error, &warning, modelPath);

    if (!file) { std::cerr << "Unable to load glTF file. \n \n"; }

    if (!error.empty()) { std::cerr << error; }

    if (!warning.empty()) { std::cerr << warning; }

    if (file) {
        fillImages(gltf);
        fillMaterials(gltf);
        fillTextures(gltf);
        const tinygltf::Scene& scene = gltf.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = gltf.nodes[scene.nodes[i]];
            fillNode(node, gltf, nullptr);
        }
    }

    
};

void Model::fillTextures(tinygltf::Model& model) {
    textures.resize(model.textures.size());
    for (size_t t = 0; t < model.textures.size(); t++) {
        textures[t].imageIndex = model.textures[t].source;
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

void Model::findComponent(const tinygltf::Accessor& accessor, const tinygltf::Buffer& buffer, 
    const tinygltf::BufferView& view) {

    //uint32_t offset = static_cast<uint32_t>(vertices.size());;
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

void Model::fillNode(const tinygltf::Node& iNode, const tinygltf::Model& model, Node* parent) {
    Node node{};
    node.matrix = glm::mat4(1.0f);

    if (iNode.translation.size() == 3) {
        node.matrix = glm::translate(node.matrix, glm::vec3(glm::make_vec3(iNode.translation.data())));
    }
    if (iNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(iNode.rotation.data());
        node.matrix *= glm::mat4(q);
    }
    if (iNode.scale.size() == 3) {
        node.matrix = glm::scale(node.matrix, glm::vec3(glm::make_vec3(iNode.scale.data())));
    }
    if (iNode.matrix.size() == 16) {
        node.matrix = glm::make_mat4x4(iNode.matrix.data());
    };

    if (iNode.children.size() > 0) {
        for (size_t i = 0; i < iNode.children.size(); i++) {
            fillNode(model.nodes[iNode.children[i]], model, &node);
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

void Model::drawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node& iNode, uint32_t& firstIndex, uint32_t& vertexOffset) {
    if (iNode.mesh.primitives.size() > 0) {
        for (Model::Primitive& prim : iNode.mesh.primitives) {
            if (prim.indexCount > 0) {
                Texture texture = textures[materials[prim.materialIndex].baseColorTextureIndex];
                vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), &texture.imageIndex);
                vkCmdDrawIndexed(commandBuffer, prim.indexCount, 1, firstIndex, vertexOffset, 0);
                firstIndex += prim.indexCount;
                vertexOffset += prim.vertexCount;
            }
        }
    }

    for (auto& child : iNode.children) {
        drawNode(commandBuffer, pipelineLayout, child, firstIndex, vertexOffset);
    }
}