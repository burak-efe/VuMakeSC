#pragma once
#include <vector>
#include "Common.h"
#include "VuMaterial.h"
#include "VuUtils.h"
#include "VuConfig.h"

namespace Vu {

    struct VuGraphicsShaderCreateInfo {
        VkPipelineCache pipelineCache;
        VkRenderPass renderPass;
    };

    struct VuShader {

    private:
    public:
        VuGraphicsShaderCreateInfo lastCreateInfo{};
        std::vector<VuMaterial> materials;


        void initAsGraphicsShader(const VuGraphicsShaderCreateInfo& createInfo) {
            lastCreateInfo = createInfo;
        }

        void uninit() {
            for (auto& material: materials) {
                material.uninit();
            }
        }

        //returns material Index
        uint32 createMaterial() {
            VuMaterial material;
            material.init({lastCreateInfo.pipelineCache, lastCreateInfo.renderPass});
            materials.push_back(material);
            return materials.capacity() - 1;
        }
    };
}
