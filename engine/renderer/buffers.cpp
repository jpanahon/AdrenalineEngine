/*
    buffers.cpp
    Adrenaline Engine

    This file has the declarations of buffers.h
*/

#include "buffers.h"

void Adren::Buffers::createModelBuffers(std::vector<Model*>& models, VkCommandPool& commandPool) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (Model* model : models) {
        indices.insert(indices.end(), model->indices.begin(), model->indices.end());
        vertices.insert(vertices.end(), model->vertices.begin(), model->vertices.end());
    }

    vertex.size = sizeof(vertices[0]) * vertices.size();
    Buffer vStaging;
    createBuffer(allocator, vertex.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vStaging, VMA_MEMORY_USAGE_AUTO);

    vmaMapMemory(allocator, vStaging.memory, &vStaging.mapped);
    memcpy(vStaging.mapped, vertices.data(), (size_t)vertex.size);
    vmaUnmapMemory(allocator, vStaging.memory);

    createBuffer(allocator, vertex.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex, VMA_MEMORY_USAGE_AUTO);

    copyBuffer(vStaging.buffer, vertex.buffer, vertex.size, commandPool);

    vmaDestroyBuffer(allocator, vStaging.buffer, vStaging.memory);

    index.size = sizeof(indices[0]) * indices.size();

    Buffer iStaging;
    createBuffer(allocator, index.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, iStaging, VMA_MEMORY_USAGE_AUTO);

    vmaMapMemory(allocator, iStaging.memory, &iStaging.mapped);
    memcpy(iStaging.mapped, indices.data(), (size_t)index.size);
    vmaUnmapMemory(allocator, iStaging.memory);

    createBuffer(allocator, index.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index, VMA_MEMORY_USAGE_AUTO);
    copyBuffer(iStaging.buffer, index.buffer, index.size, commandPool);

    vmaDestroyBuffer(allocator, iStaging.buffer, iStaging.memory);

#ifdef DEBUG
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_BUFFER, (uint64_t)vertex.buffer, "VERTEX BUFFER");
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_BUFFER, (uint64_t)index.buffer, "INDEX BUFFER");
#endif
}

void Adren::Buffers::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool& commandPool) {
    VkCommandBuffer commandBuffer = Adren::Tools::beginSingleTimeCommands(device, commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    Adren::Tools::endSingleTimeCommands(commandBuffer, device, graphicsQueue, commandPool);
}

void Adren::Buffers::createBuffer(VmaAllocator& allocator, VkDeviceSize& size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer, VmaMemoryUsage vmaUsage) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = vmaUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    vmaAllocInfo.preferredFlags = properties;

    vmaCreateBuffer(allocator, &bufferInfo, &vmaAllocInfo, &buffer.buffer, &buffer.memory, nullptr);
}

void Adren::Buffers::createUniformBuffers(std::vector<VkImage>& images, std::vector<Model*>& models) {
    VkPhysicalDeviceProperties gpuProperties{};
    vkGetPhysicalDeviceProperties(gpu, &gpuProperties);
    VkDeviceSize minUboAlignment = gpuProperties.limits.minUniformBufferOffsetAlignment;
    dynamicUniform.align = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        dynamicUniform.align = (dynamicUniform.align + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }
    
    uint32_t modelSize = 0;
    
    for (Model* model : models) {
        modelSize += model->modelSize;
    }

    Adren::Tools::checkSize("Model Size: ", modelSize);
    
    dynamicUniform.size = dynamicUniform.align * modelSize;
    uboData.model = (glm::mat4*)Adren::Tools::alignedAlloc(dynamicUniform.size, dynamicUniform.align);
    assert(uboData.model);

    createBuffer(allocator, dynamicUniform.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, dynamicUniform, VMA_MEMORY_USAGE_AUTO);
    vmaMapMemory(allocator, dynamicUniform.memory, &dynamicUniform.mapped);
    memcpy(dynamicUniform.mapped, uboData.model, dynamicUniform.size);

#ifdef DEBUG
    Adren::Tools::label(instance, device, VK_OBJECT_TYPE_BUFFER, (uint64_t)dynamicUniform.buffer, "DYNAMIC UNIFORM");
#endif
}

void Adren::Buffers::updateDynamicUniformBuffer(std::vector<Model*>& models) {
    std::vector<glm::mat4> matrices;

    for (Model* model : models) {
        matrices.insert(matrices.end(), model->matrices.begin(), model->matrices.end());
    }
    
    Adren::Tools::checkSize("Matrices: ", matrices.size());
    
    VkDeviceSize alignment = dynamicUniform.align * matrices.size();
    memcpy(dynamicUniform.mapped, matrices.data(), alignment);
    vmaFlushAllocation(allocator, dynamicUniform.memory, alignment, sizeof(glm::mat4));
}

void Adren::Buffers::cleanup() {
    if (uboData.model) { Adren::Tools::alignedFree(uboData.model); }

    vmaDestroyBuffer(allocator, vertex.buffer, vertex.memory);
    vmaDestroyBuffer(allocator, index.buffer, index.memory);

    vmaUnmapMemory(allocator, dynamicUniform.memory);
    vmaDestroyBuffer(allocator, dynamicUniform.buffer, dynamicUniform.memory);
    dynamicUniform.mapped = nullptr;
}
