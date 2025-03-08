#pragma once
#include "Common.h"
#include "VuBuffer.h"
#include "VuTypes.h"

namespace Vu {

    struct VuMaterialDataPool {
    private:
        static constexpr VkDeviceSize BLOCK_SIZE  = 64;
        static constexpr VkDeviceSize BLOCK_COUNT = 1024;

        //inline static VkDeviceAddress    deviceAddress;
        inline static VuHandle<VuBuffer> globalMaterialDataBuffer;
        inline static uint32             allocCounter = 0;
        inline static std::stack<uint32> freeList;

    public:
        static void init() {
            globalMaterialDataBuffer.createHandle()->init({
                .length = BLOCK_COUNT,
                .strideInBytes = BLOCK_SIZE,
                .usageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                .memoryPropertyFlags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                .createFlags = 0U
            });

            globalMaterialDataBuffer.get()->map();
            //deviceAddress = globalMaterialDataBuffer.get()->getDeviceAddress();
            uint32 index = globalMaterialDataBuffer.index;
            VuResourceManager::registerStorageBuffer(index, *globalMaterialDataBuffer.get());

        }

        // static VkDeviceAddress mapAddressToBufferDeviceAddress(GPU_PBR_MaterialData* ptr) {
        //     uint32 offset = (uint64) globalMaterialDataBuffer.get()->mapPtr - (uint64) ptr;
        //     return deviceAddress + offset;
        // }

        static uint32 allocBlock() {
            uint32 index = 0U;
            if (!freeList.empty()) {
                index = freeList.top();
                freeList.pop();
            } else {
                index = allocCounter;
                allocCounter++;
            }
            return index;
        }

        static void freeBlock(uint32 blockIndex) {
            freeList.push(blockIndex);
        }

        static GPU_PBR_MaterialData* getMaterialData(uint32 blockIndex) {
            uint64 offset = blockIndex * BLOCK_SIZE;
            return (GPU_PBR_MaterialData *) ((uint64) globalMaterialDataBuffer.get()->mapPtr + offset);
        }

        static void uninit() {
            globalMaterialDataBuffer.get()->unmap();
            globalMaterialDataBuffer.destroyHandle();
        }
    };
}
