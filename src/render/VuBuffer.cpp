#include "VuBuffer.h"

#include "VuCtx.h"
#include "VuDevice.h"

namespace Vu {


    void VuBuffer::initGlobalStagingBuffer(const VuBufferCreateInfo& createInfo) {
        globalStagingBuffer = new VuBuffer();
        globalStagingBuffer->init(createInfo);
        globalStagingBuffer->map();
    }

    void VuBuffer::init(const VuBufferCreateInfo& info) {

        VkCheck(createBuffer(ctx::vuDevice->device, ctx::vuDevice->physicalDevice, (info.length * info.strideInBytes), info.usageFlags,
                             info.memoryPropertyFlags, buffer, memory));
        createInfo = info;
        stride     = info.strideInBytes;
        lenght     = info.length;
    }

    void VuBuffer::uninit() {
        if (mapPtr != nullptr) {
            unmap();
        }

        vkDestroyBuffer(ctx::vuDevice->device, buffer,nullptr);

    }

    void VuBuffer::map() {
        VkCheck(vkMapMemory(ctx::vuDevice->device, memory, 0,VK_WHOLE_SIZE, 0, &mapPtr));
    }

    void VuBuffer::unmap() {
        vkUnmapMemory(ctx::vuDevice->device, memory);
        mapPtr = nullptr;
    }

    VkDeviceAddress VuBuffer::getDeviceAddress() const {
        VkBufferDeviceAddressInfo deviceAdressInfo{};
        deviceAdressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAdressInfo.buffer = buffer;

        VkDeviceAddress address = vkGetBufferDeviceAddress(ctx::vuDevice->device, &deviceAdressInfo);
        return address;
    }

    VkResult VuBuffer::setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offset) {
        uint64 ptr = reinterpret_cast<uint64_t>(mapPtr) + offset;
        memcpy((void *) ptr, data, byteSize);
        return VK_SUCCESS;
    }

    VkDeviceSize VuBuffer::getSizeInBytes() {
        return lenght * stride;
    }

    std::span<uint8> VuBuffer::getSpan(VkDeviceSize start, VkDeviceSize byteLength) {
        auto* ptr = static_cast<uint8 *>(mapPtr);
        ptr += start;
        std::span span(ptr, byteLength);
        return span;
    }

    void VuBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = ctx::vuDevice->BeginSingleTimeCommands();
        VkBufferCopy    copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size      = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        ctx::vuDevice->EndSingleTimeCommands(commandBuffer);
    }

    VkDeviceSize VuBuffer::alignedSize(VkDeviceSize value, VkDeviceSize alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    VkResult VuBuffer::createBuffer(
        VkDevice              device,
        VkPhysicalDevice      physicalDevice,
        VkDeviceSize          size,
        VkBufferUsageFlags    usage,
        VkMemoryPropertyFlags properties,
        VkBuffer&             buffer,
        VkDeviceMemory&       bufferMemory) {

        // Create buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
        if (result != VK_SUCCESS) {
            return result;
        }

        // Get memory requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        // Find suitable memory type
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        uint32_t memoryTypeIndex = UINT32_MAX;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                memoryTypeIndex = i;
                break;
            }
        }

        if (memoryTypeIndex == UINT32_MAX) {
            vkDestroyBuffer(device, buffer, nullptr);
            return VK_ERROR_FEATURE_NOT_PRESENT; // No suitable memory type found
        }

        // Allocate memory

        VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .pNext = nullptr,
            .flags = (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0 ? VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT : 0,
            .deviceMask = 0,
        };
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext           = &memoryAllocateFlagsInfo;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS) {
            vkDestroyBuffer(device, buffer, nullptr);
            return result;
        }

        // Bind buffer with memory
        result = vkBindBufferMemory(device, buffer, bufferMemory, 0);
        if (result != VK_SUCCESS) {
            //vkFreeMemory(device, bufferMemory, nullptr);
            vkDestroyBuffer(device, buffer, nullptr);
            return result;
        }

        return VK_SUCCESS;
    }
}
