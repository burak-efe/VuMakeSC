#include "VuResourceManager.h"

#include "VuConfig.h"
#include "VuCtx.h"
#include "VuDevice.h"

void Vu::VuResourceManager::init(const VuBindlessConfigInfo& info) {
    bufferOfStorageBuffer.init({.length = info.storageBufferCount, .strideInBytes = sizeof(uint64)});
    bufferOfStorageBuffer.map();

    writeStorageBuffer(bufferOfStorageBuffer, config::BINDLESS_CONFIG_INFO.storageBufferBinding);
}

void Vu::VuResourceManager::uninit() {
    bufferOfStorageBuffer.uninit();
}

void Vu::VuResourceManager::registerStorageBuffer(uint32 writeIndex, const VuBuffer& buffer) {
    VkDeviceAddress address = buffer.getDeviceAddress();
    VkCheck(bufferOfStorageBuffer.setData(&address, sizeof(VkDeviceAddress), writeIndex * sizeof(VkDeviceAddress)));
}

void Vu::VuResourceManager::writeStorageBuffer(const VuBuffer& buffer, uint32 binding) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer.buffer;
    bufferInfo.range  = buffer.lenght * buffer.stride;

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = ctx::vuDevice->globalDescriptorSets[i];
        descriptorWrite.dstBinding      = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo     = &bufferInfo;
        vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &descriptorWrite, 0, nullptr);
    }
}

void Vu::VuResourceManager::writeSampledImageToGlobalPool(uint32 writeIndex, const VkImageView& imageView) {

    VkDescriptorImageInfo imageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = ctx::vuDevice->globalDescriptorSets[i];
        descriptorWrite.dstBinding      = config::BINDLESS_CONFIG_INFO.sampledImageBinding;
        descriptorWrite.dstArrayElement = writeIndex;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo      = &imageInfo;
        vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &descriptorWrite, 0, nullptr);
    }
}

void Vu::VuResourceManager::writeSamplerToGlobalPool(uint32 writeIndex, const VkSampler& sampler) {

    VkDescriptorImageInfo imageInfo{
        .sampler = sampler,
    };

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet samplerWrite{};
        samplerWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerWrite.dstSet          = ctx::vuDevice->globalDescriptorSets[i];
        samplerWrite.dstBinding      = config::BINDLESS_CONFIG_INFO.samplerBinding;
        samplerWrite.dstArrayElement = writeIndex;
        samplerWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
        samplerWrite.descriptorCount = 1;
        samplerWrite.pImageInfo      = &imageInfo;
        vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &samplerWrite, 0, nullptr);
    }
}

void Vu::VuResourceManager::writeUBO_ToGlobalPool(uint32 writeIndex, uint32 setIndex, const VuBuffer& buffer) {

    VkDescriptorBufferInfo bufferInfo{
        .buffer = buffer.buffer,
        .offset = 0,
        .range = sizeof(GPU_FrameConst)
    };

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet          = ctx::vuDevice->globalDescriptorSets[setIndex];
    descriptorWrite.dstBinding      = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo     = &bufferInfo;

    vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &descriptorWrite, 0, nullptr);
}
