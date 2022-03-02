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

    bool file = tinyGLTF.LoadASCIIFromFile(&model, &error, &warning, modelPath);

    if (!file) { std::cerr << "Unable to load glTF file. \n \n"; }

    if (!error.empty()) { std::cerr << error; }

    if (!warning.empty()) { std::cerr << warning; }

    for (auto& mesh : model.meshes) {
        for (auto& prim : mesh.primitives) {
            tinygltf::Accessor& vertexAcc = model.accessors[prim.attributes["POSITION"]];
            tinygltf::BufferView& vertBufferView = model.bufferViews[vertexAcc.bufferView];
            tinygltf::Buffer& vertBuffer = model.buffers[vertBufferView.buffer];
            float* modelVert = reinterpret_cast<float*>(&vertBuffer.data[vertBufferView.byteOffset + vertexAcc.byteOffset]);

            tinygltf::Accessor& texAcc = model.accessors[prim.attributes["TEXCOORD_0"]];
            tinygltf::BufferView& texBufferView = model.bufferViews[texAcc.bufferView];
            tinygltf::Buffer& texBuffer = model.buffers[texBufferView.buffer];
            float* modelTex = reinterpret_cast<float*>(&texBuffer.data[texBufferView.byteOffset + texAcc.byteOffset]);

            tinygltf::Accessor& indexAcc = model.accessors[prim.indices];
            tinygltf::BufferView& indBufferView = model.bufferViews[indexAcc.bufferView];
            tinygltf::Buffer& indBuffer = model.buffers[indBufferView.buffer];
            uint32_t* modelInd = reinterpret_cast<uint32_t*>(&indBuffer.data[indexAcc.byteOffset + indBufferView.byteOffset]);

            Primitive primitive{};
            primitive.vertexCount = vertexAcc.count;
            primitive.indexCount = indexAcc.count;


            for (size_t v = 0; v < vertexAcc.count; v++) {
                Vertex vertex{};

                vertex.pos = glm::vec4(glm::make_vec3(&modelVert[v * 3]), 1.0f);

                vertex.texCoord = modelTex ? glm::make_vec2(&modelTex[v * 2]) : glm::vec3(0.0f);

                vertex.color = glm::vec3(1.0f);
                vertices.push_back(vertex);
            }

            for (uint32_t x = 0; x < indexAcc.count; x++) {
                indices.push_back(modelInd[x]);
            }

            primitive.firstIndex = static_cast<uint32_t>(indices.size());
            primitive.firstVertex = static_cast<uint32_t>(vertices.size());
            primitive.materialIndex = prim.material;
            primitives.push_back(primitive);
        }
        textures.resize(model.textures.size());
        for (size_t t = 0; t < model.textures.size(); t++) {
            textures[t].imageIndex = model.textures[t].source;
        }

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
                for (size_t i = 0; i < image.width * image.height; ++i) {
                    memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                    rgba += 4;
                    rgb += 3;
                }
            } else {
                imageBuffer = &image.image[0];
                imageBufferSize = image.image.size();
            }

            images[i].height = image.height;
            images[i].width = image.width;
            images[i].buffer = imageBuffer;
            images[i].bufferSize = imageBufferSize;
        }
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
};