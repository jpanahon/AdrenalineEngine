/* 
	model.cpp
	Adrenaline Engine

	This defines the models.
*/

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#include "model.h"
#include "tools.h"

#include "../adrenaline.h" // This is where STB_IMAGE_IMPLEMENTATION is defined.

Adren::Model::Model(std::string_view modelPath) {
    std::cout << "Loading " << modelPath << std::endl;

    {
        fastgltf::Parser parser(fastgltf::Extensions::KHR_mesh_quantization);

        auto path = std::filesystem::path{ modelPath };

        constexpr auto options =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
            fastgltf::Options::LoadGLBBuffers |
            fastgltf::Options::LoadExternalBuffers |
            fastgltf::Options::LoadExternalImages;

        fastgltf::GltfDataBuffer data;
        data.loadFromFile(modelPath);

        auto type = fastgltf::determineGltfFileType(&data);
        fastgltf::Expected<fastgltf::Asset> asset(fastgltf::Error::None);

        if (type == fastgltf::GltfType::glTF) {
            asset = parser.loadGLTF(&data, path.parent_path(), options);
        } else if (type == fastgltf::GltfType::GLB) {
            asset = parser.loadBinaryGLTF(&data, path.parent_path(), options);
        }

        if (asset.error() != fastgltf::Error::None) {
            Adren::Tools::log("Failed to open gltf file.");
        }

        gltfModel = std::move(asset.get());
        Adren::Tools::log("GLTF Loaded");

        for (auto& image : gltfModel.images) {
            loadImages(image);
        }

        for (auto& texture : gltfModel.textures) {
            loadTextures(texture);
        }

        for (auto& material : gltfModel.materials) {
            loadMaterials(material);
        }

        for (auto& mesh : gltfModel.meshes) {
            loadMesh(mesh);
        }

        std::size_t sceneIndex = 0;

        if (gltfModel.defaultScene.has_value())
            sceneIndex = gltfModel.defaultScene.value();

        auto& scene = gltfModel.scenes[sceneIndex];
        for (auto& node : scene.nodeIndices) {
            countMeshes(modelSize, node);
            countMatrices(matrices, node, glm::mat4(1.0f));
        }
    }
}

void Adren::Model::countMeshes(uint32_t& num, size_t index) {
    fastgltf::Node node = gltfModel.nodes[index];
    num++;
   
    for (const auto& child : node.children) {
        countMeshes(num, child);
    }
}

void Adren::Model::countMatrices(std::vector<glm::mat4>& matrices, size_t index, glm::mat4 matrix) {
    fastgltf::Node node = gltfModel.nodes[index];
    matrices.push_back(getTransformMatrix(node, matrix));

    for (const auto& child : node.children) {
        countMatrices(matrices, child, matrix);
    }
}


glm::mat4 Adren::Model::getTransformMatrix(const fastgltf::Node& node, glm::mat4x4& base) {
    if (const auto* pMatrix = std::get_if<fastgltf::Node::TransformMatrix>(&node.transform)) {
        return base * glm::mat4x4(glm::make_mat4x4(pMatrix->data()));
    }
    else if (const auto* pTransform = std::get_if<fastgltf::Node::TRS>(&node.transform)) {
        return base
            * glm::translate(glm::mat4(1.0f), glm::make_vec3(pTransform->translation.data()))
            * glm::toMat4(glm::make_quat(pTransform->rotation.data()))
            * glm::scale(glm::mat4(1.0f), glm::make_vec3(pTransform->scale.data()));
    }
    else {
        return base;
    }
}

bool Adren::Model::loadImages(fastgltf::Image& image) {
    glTFImage gltfImage{};
    
    std::visit(fastgltf::visitor{
        [](auto& arg) {},
        [&](fastgltf::sources::URI& filePath) {
            assert(filePath.fileByteOffset == 0);
            assert(filePath.uri.isLocalPath());
            int width, height, nrChannels;

            const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
            gltfImage = glTFImage {
                .buffer = data,
                .bufferSize = static_cast<VkDeviceSize>(width * height * 4),
                .height = height,
                .width = width
            };
        },
        [&](fastgltf::sources::Vector& vector) {
            int width, height, nrChannels;
            unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
            gltfImage = glTFImage {
                .buffer = data,
                .bufferSize = static_cast<VkDeviceSize>(width * height * 4),
                .height = height,
                .width = width
            };
        },
        [&](fastgltf::sources::BufferView& view) {
            auto& bufferView = gltfModel.bufferViews[view.bufferViewIndex];
            auto& buffer = gltfModel.buffers[bufferView.bufferIndex];
            std::visit(fastgltf::visitor {
                [](auto& arg) {},
                [&](fastgltf::sources::Vector& vector) {
                    int width, height, nrChannels;
                    unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset, static_cast<int>(bufferView.byteLength), &width, &height, &nrChannels, 4);
                    gltfImage = glTFImage {
                        .buffer = data,
                        .bufferSize = static_cast<VkDeviceSize>(width * height * 4),
                        .height = height,
                        .width = width
                    };
                }
            }, buffer.data);
        },
    }, image.data);

    images.emplace_back(gltfImage);

    return true;
}

bool Adren::Model::loadMaterials(fastgltf::Material& material) {
    Material newMaterial{};
    newMaterial.baseColorFactor = glm::make_vec4(material.pbrData.baseColorFactor.data());

    if (material.pbrData.baseColorTexture.has_value()) {
        newMaterial.baseColorTextureIndex = (material.pbrData.baseColorTexture.value()).textureIndex;
    }

    materials.emplace_back(newMaterial);
    return true;
}

bool Adren::Model::loadTextures(fastgltf::Texture& texture) {
    Texture newTex{};
    newTex.index = texture.imageIndex.value();
    textures.push_back(newTex);
    return true;
}

bool Adren::Model::loadMesh(fastgltf::Mesh& mesh) {
    Mesh newMesh{};
    newMesh.primitives.resize(mesh.primitives.size());

    for (auto prim = mesh.primitives.begin(); prim != mesh.primitives.end(); ++prim) {
        auto index = std::distance(mesh.primitives.begin(), prim);
        auto& primitive = newMesh.primitives[index];

        std::vector<Vertex> tempVertices;

        auto pos = prim->findAttribute("POSITION");

        if (pos == prim->attributes.end()) continue;

        if (!prim->indicesAccessor.has_value()) return false;

        if (prim->indicesAccessor.has_value()) {
            auto& iAccessor = gltfModel.accessors[prim->indicesAccessor.value()];
            auto& iBufferView = gltfModel.accessors[iAccessor.bufferViewIndex.value()];

            fastgltf::iterateAccessorWithIndex<uint32_t>(gltfModel, iAccessor, [&](uint32_t index, size_t idx) {
                indices.push_back(index);
            });

            primitive.indexCount = iAccessor.count;
            primitive.firstIndex = static_cast<uint32_t>(iAccessor.byteOffset + iBufferView.byteOffset) / fastgltf::getElementByteSize(iAccessor.type, iAccessor.componentType);
        }

        if (pos != prim->attributes.end()) {
            auto& vAccessor = gltfModel.accessors[pos->second];
            if (!vAccessor.bufferViewIndex.has_value()) continue;
            
            tempVertices.resize(vAccessor.count);

            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltfModel, vAccessor, [&](glm::vec3 position, size_t idx) {
                tempVertices[idx].pos = position;
                tempVertices[idx].color = glm::vec3(1.0f);
            });

            primitive.vertexCount = vAccessor.count;
        }

        auto texCoord = prim->findAttribute("TEXCOORD_0");

        if (texCoord != prim->attributes.end()) {
            auto& tAccessor = gltfModel.accessors[texCoord->second];
            if (!tAccessor.bufferViewIndex.has_value()) continue;

            fastgltf::iterateAccessorWithIndex<glm::vec2>(gltfModel, tAccessor, [&](glm::vec2 position, size_t idx) {
                tempVertices[idx].texCoord = position;
            });

            primitive.materialIndex = prim->materialIndex.value();
        }

        vertices.insert(vertices.end(), tempVertices.begin(), tempVertices.end());
    }

    meshes.emplace_back(newMesh);
    return true;
}

void Adren::Model::drawMesh(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout, VkDescriptorSet& set, Offset& offset) {
    Mesh mesh = meshes[index];

    for (auto& prim : mesh.primitives) {
        if (prim.indexCount > 0) {
            Texture texture = textures[materials[prim.materialIndex].baseColorTextureIndex];
            const auto index = texture.index + offset.texture;
            vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set, 1, &offset.dynamic);
            vkCmdPushConstants(buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(index), &index);
            vkCmdDrawIndexed(buffer, prim.indexCount, 1, offset.index, offset.vertex, 0);
            offset.index += prim.indexCount;
            offset.vertex += prim.vertexCount;
        }
    }
}

void Adren::Model::drawNode(size_t index, VkCommandBuffer& buffer, VkPipelineLayout& layout, VkDescriptorSet& set, Offset& offset) {
    auto& node = gltfModel.nodes[index];

    if (node.meshIndex.has_value()) {
        drawMesh(node.meshIndex.value(), buffer, layout, set, offset);
    }

    for (auto& child : node.children) {
        drawNode(child, buffer, layout, set, offset);
    }
}