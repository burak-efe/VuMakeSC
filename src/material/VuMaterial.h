#pragma once


#include "Common.h"
#include "VuGraphicsPipeline.h"
#include "VuMesh.h"
#include "VuTypes.h"
#include "VuMaterialDataPool.h"


namespace Vu {
    struct VuMaterialCreateInfo {
        VkPipelineCache pipelineCache;
        VkRenderPass renderPass;
    };

    struct VuMaterial {
        VuGraphicsPipeline vuPipeline;
        uint32 index;

        void init(const VuMaterialCreateInfo& createInfo) {
            vuPipeline.initGraphicsPipeline(
                ctx::vuDevice->globalPipelineLayout,
                createInfo.pipelineCache,
                createInfo.renderPass
            );
            index = VuMaterialDataPool::allocBlock();
        }

        void uninit() {
            vuPipeline.Dispose();
            VuMaterialDataPool::freeBlock(index);
        }

        GPU_PBR_MaterialData* getPbrMaterialData() {
            return Vu::VuMaterialDataPool::getMaterialData(index);
        }

        void bindPipeline(const VkCommandBuffer& commandBuffer) const {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
        }
    };
}
