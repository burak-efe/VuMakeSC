#pragma once

#include "Common.h"

namespace Vu {

    struct VuBufferCreateInfo {
        VkDeviceSize          length              = 1U;
        VkDeviceSize          strideInBytes       = 4U;
        VkBufferUsageFlags    usageFlags          = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        VkBufferCreateFlags   createFlags         = 0U;
    };

    struct VuBuffer {
    public:
        VuBufferCreateInfo createInfo;
        VkBuffer           buffer;
        VkDeviceMemory     memory;
        VkDeviceSize       lenght;
        VkDeviceSize       stride;
        void*              mapPtr;

        static inline VuBuffer* globalStagingBuffer;

        static void initGlobalStagingBuffer(const VuBufferCreateInfo& createInfo);

        void init(const VuBufferCreateInfo& info);

        void uninit();

        void map();

        void unmap();

        [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

        VkResult setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offset = 0U);

        VkDeviceSize getSizeInBytes();

        std::span<uint8> getSpan(VkDeviceSize start, VkDeviceSize byteLength);

        static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        static VkDeviceSize alignedSize(VkDeviceSize value, VkDeviceSize alignment);

        static VkResult createBuffer(
            VkDevice              device,
            VkPhysicalDevice      physicalDevice,
            VkDeviceSize          size,
            VkBufferUsageFlags    usage,
            VkMemoryPropertyFlags properties,
            VkBuffer&             buffer,
            VkDeviceMemory&       bufferMemory);
    };
}
