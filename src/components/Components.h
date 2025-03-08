#pragma once

#include "Common.h"
#include "VuResourceManager.h"


namespace Vu {
    struct VuMesh;
    struct VuShader;

    struct MeshRenderer {
        VuMesh* mesh;
        VuHandle<VuShader> shader;
        uint32 materialIndex;
    };


    struct Spinn {
        float3 axis = float3(0, 1, 0);
        float angle = glm::radians(0.f);
    };
}
