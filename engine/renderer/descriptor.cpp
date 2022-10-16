/*
    descriptor.cpp
    Adrenaline Engine

    Everything related to descriptor sets are defined here.
*/
#include "descriptor.h"
#include "info.h"

void Adren::Descriptor::createLayout(std::vector<Model*>& models) {
    VkDescriptorSetLayoutBinding uboBinding = Adren::Info::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutBinding dynamicUboBinding = Adren::Info::uboLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1);

    VkDescriptorSetLayoutBinding samplerBinding = Adren::Info::samplerLayoutBinding();

    VkDescriptorSetLayoutBinding textureBinding = Adren::Info::textureLayoutBinding(2048);


    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {uboBinding, dynamicUboBinding, samplerBinding, textureBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());

    VkDescriptorBindingFlags flags[4];
    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 0;
    flags[3] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.bindingCount = static_cast<uint32_t>(bindings.size());
    bindingFlags.pBindingFlags = flags;

    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = &bindingFlags;

    Adren::Tools::vibeCheck("DESCRIPTOR SET LAYOUT", vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
}

void Adren::Descriptor::createPool(std::vector<VkImage>& images) {
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; poolSizes[0].descriptorCount = 100000;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER; poolSizes[1].descriptorCount = 100000;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; poolSizes[2].descriptorCount = 100000;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; poolSizes[3].descriptorCount = 100000;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(images.size());
    //poolInfo.maxSets = 200;

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

void Adren::Descriptor::createSets(std::vector<Model::Texture>& textures, std::vector<VkImage>& images, Buffer& cam) {
    size_t textureSize = textures.size();
    uint32_t setCount = static_cast<uint32_t>(images.size());
    std::vector<VkDescriptorSetLayout> layouts(setCount, layout);
    
    uint32_t counts[4];
    for (int c = 0; c < 4; c++) { counts[c] = textureSize; }

    VkDescriptorSetVariableDescriptorCountAllocateInfo setCounts{};
    setCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    setCounts.descriptorSetCount = setCount;
    setCounts.pDescriptorCounts = counts;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = setCount;
    allocInfo.pSetLayouts = layouts.data();
    allocInfo.pNext = &setCounts;

    sets.resize(setCount);
    Adren::Tools::vibeCheck("ALLOCATED DESCRIPTOR SETS", vkAllocateDescriptorSets(device, &allocInfo, sets.data()));

    for (size_t i = 0; i < sets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cam.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraObject);

        VkDescriptorBufferInfo dynamicBufferInfo{};
        dynamicBufferInfo.buffer = buffers.dynamicUniform.buffer;
        dynamicBufferInfo.offset = 0;
        dynamicBufferInfo.range = sizeof(glm::mat4);

        VkSamplerCreateInfo sampInfo = Adren::Info::samplerInfo();
        Adren::Tools::vibeCheck("CREATE SAMPLER", vkCreateSampler(device, &sampInfo, nullptr, &sampler));

        VkDescriptorImageInfo samplerInfo{};
        samplerInfo.sampler = sampler;
        std::vector<VkDescriptorImageInfo> imageInfo;
        for (uint32_t t = 0; t < textureSize; t++) {
            VkDescriptorImageInfo info;
            info.sampler = sampler;
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = textures[t].view;
            imageInfo.push_back(info);
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
        dWrites[3].pImageInfo = imageInfo.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(dWrites.size()), dWrites.data(), 0, nullptr);
    }
}

void Adren::Descriptor::cleanup() {
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
    vkDestroyDescriptorPool(device, pool, nullptr);
    vkDestroySampler(device, sampler, nullptr);
}
