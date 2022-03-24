/*
    descriptor.cpp
    Adrenaline Engine

    Everything related to descriptor sets are defined here.
*/
#include "descriptor.h"
#include "info.h"

void Adren::Descriptor::createLayout(std::vector<Model>& models) {
    std::vector<Model::Texture> textures;
    for (Model m : models) {
        for (const Model::Texture& t : m.textures) {
            textures.push_back(t);
        }
    }

    VkDescriptorSetLayoutBinding uboBinding = Adren::Info::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutBinding dynamicUboBinding = Adren::Info::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1);

    VkDescriptorSetLayoutBinding samplerBinding = Adren::Info::samplerLayoutBinding();

    VkDescriptorSetLayoutBinding textureBinding = Adren::Info::textureLayoutBinding(textures.size());

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {uboBinding, dynamicUboBinding, samplerBinding, textureBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    Adren::Tools::vibeCheck("DESCRIPTOR SET LAYOUT", vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
}

void Adren::Descriptor::createPool(std::vector<VkImage>& images) {
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; poolSizes[0].descriptorCount = 1000;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER; poolSizes[1].descriptorCount = 1000;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; poolSizes[2].descriptorCount = 1000;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; poolSizes[3].descriptorCount = 1000;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(images.size());

    Adren::Tools::vibeCheck("DESCRIPTOR POOL", vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
}

void Adren::Descriptor::fillWrites(std::array<VkWriteDescriptorSet, 4>& write, int index, VkDescriptorSet& dSet, int binding, VkDescriptorType type, size_t& count) {
    write[index].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[index].dstSet = dSet;
    write[index].dstBinding = binding;
    write[index].dstArrayElement = 0;
    write[index].descriptorType = type;
    write[index].descriptorCount = count;
}

void Adren::Descriptor::createSets(std::vector<Model::Texture>& textures, std::vector<VkImage>& images) {
    size_t textureSize = textures.size();
    std::vector<VkDescriptorSetLayout> layouts(images.size(), layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(images.size());
    allocInfo.pSetLayouts = layouts.data();

    sets.resize(images.size());
    Adren::Tools::vibeCheck("ALLOCATED DESCRIPTOR SETS", vkAllocateDescriptorSets(device, &allocInfo, sets.data()));

    for (size_t i = 0; i < sets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffers.uniform.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorBufferInfo dynamicBufferInfo{};
        dynamicBufferInfo.buffer = buffers.dynamicUniform[i].buffer;
        dynamicBufferInfo.offset = 0;
        dynamicBufferInfo.range = sizeof(glm::mat4);

        VkSamplerCreateInfo sampInfo = Adren::Info::samplerInfo();
        Adren::Tools::vibeCheck("CREATE SAMPLER", vkCreateSampler(device, &sampInfo, nullptr, &sampler));

        VkDescriptorImageInfo samplerInfo{};
        samplerInfo.sampler = sampler;
        VkDescriptorImageInfo* imageInfo;
        imageInfo = new VkDescriptorImageInfo[textureSize];
        for (uint32_t t = 0; t < textureSize; t++) {
            imageInfo[t].sampler = sampler;
            imageInfo[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[t].imageView = textures[t].view;
        }

        std::array<VkWriteDescriptorSet, 4> dWrites{};

        size_t count = 1;
        fillWrites(dWrites, 0, sets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count);
        dWrites[0].pBufferInfo = &bufferInfo;

        fillWrites(dWrites, 1, sets[i], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count);
        dWrites[1].pBufferInfo = &dynamicBufferInfo;

        fillWrites(dWrites, 2, sets[i], 2, VK_DESCRIPTOR_TYPE_SAMPLER, count);
        dWrites[2].pImageInfo = &samplerInfo;

        fillWrites(dWrites, 3, sets[i], 3, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, textureSize);
        dWrites[3].pImageInfo = imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(dWrites.size()), dWrites.data(), 0, nullptr);
        delete[] imageInfo;
    }
}
