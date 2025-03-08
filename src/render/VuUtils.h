#pragma once

#include <fstream>
#include "Common.h"

namespace Vu {


    inline std::vector<char> readFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t            fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    inline void createFile(const std::filesystem::path& path, std::span<const uint8_t> data) {
        std::ofstream file(path, std::ios::binary | std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        file.write(reinterpret_cast<const char *>(data.data()), data.size());

        if (!file) {
            throw std::runtime_error("Failed to write to file: " + path.string());
        }
    }


    inline VkImageCreateInfo fillImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
        VkImageCreateInfo info = {};
        info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext             = nullptr;

        info.imageType = VK_IMAGE_TYPE_2D;

        info.format = format;
        info.extent = extent;

        info.mipLevels   = 1;
        info.arrayLayers = 1;
        info.samples     = VK_SAMPLE_COUNT_1_BIT;
        info.tiling      = VK_IMAGE_TILING_OPTIMAL;
        info.usage       = usageFlags;

        return info;
    }


    inline uint32 findMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32 i = 0U; i < memProperties.memoryTypeCount; i++) {
            bool includedInFilter = typeFilter & (1 << i);
            bool hasAllProperties = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;
            if (includedInFilter && hasAllProperties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }

    inline void createPipelineLayout(const VkDevice                         device,
                                     const std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                                     const uint32                           pushConstantSizeAsByte,
                                     VkPipelineLayout&                      outPipelineLayout) {

        //push constants
        VkPushConstantRange pcRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0U,
            .size = pushConstantSizeAsByte,
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = static_cast<uint32>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1U;
        pipelineLayoutInfo.pPushConstantRanges    = &pcRange;

        VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout));
    }

}
