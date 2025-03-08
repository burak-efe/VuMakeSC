#pragma once

#include <filesystem>

#include "glm/gtx/string_cast.hpp"

#include "Camera.h"
#include "Components.h"
#include "VuMesh.h"
#include "Transform.h"
#include "VuAssetLoader.h"
#include "VuResourceManager.h"
#include "VuRenderer.h"
#include "VuShader.h"


namespace Vu {

    struct Scene0 {
    private:
        VuRenderer vuRenderer{};

        std::filesystem::path jetPath      = "assets/gltf/jet/jet.gltf";
        std::filesystem::path mountainPath = "assets/gltf/mountain/mountain.gltf";

        VuShader pbrShader{};

        Transform jetTransform      = {{0, 200, 0}, glm::quat(glm::vec3{0, 0, 0}), {1, 1, 1}};
        Transform camTransform      = {{0, 208, -15.0F}, glm::quat(glm::vec3{-0.1F, 3.1415F, 0}), {1, 1, 1}};
        Transform mountainTransform = {{0, 0, 125}, glm::quat(glm::vec3{0, 0, 0}), {100, 100, 100}};

        Camera cam{};

        std::chrono::time_point<std::chrono::steady_clock> prevTime{};

    private:
        void renderMesh(VuMesh& mesh, VuMaterial& material, Transform trs) {

            auto matIndex = material.index;
            ctx::vuRenderer->bindMaterial(material);

            GPU_PushConstant pc{
                trs.ToTRS(),
                matIndex,
                {
                    mesh.vertexBuffer.index,
                    mesh.vertexCount,
                    0
                }
            };

            ctx::vuRenderer->pushConstants(pc);
            ctx::vuRenderer->bindMesh(mesh);
            ctx::vuRenderer->drawIndexed(mesh.indexBuffer.get()->lenght);
        }


        void updateFrameConstant() {
            ctx::frameConst.view = glm::inverse(camTransform.ToTRS());
            ctx::frameConst.proj = glm::perspective(
                glm::radians(cam.fov),
                static_cast<float>(ctx::vuRenderer->swapChain.swapChainExtent.width)
                / static_cast<float>(ctx::vuRenderer->swapChain.swapChainExtent.height),
                cam.near,
                cam.far);

            ctx::frameConst.cameraPos = glm::vec4(camTransform.Position, 0);
            ctx::frameConst.cameraDir = glm::vec4(float3(cam.yaw, cam.pitch, cam.roll), 0);
            ctx::frameConst.time      = glm::vec4(ctx::time(), 0, 0, 0).x;

            ctx::vuRenderer->updateFrameConstantBuffer(ctx::frameConst);
        }

    public:
        void Run() {
            std::vector<char> pipelineCache = readFile("assets\\shaders\\pipeline_cache.bin");

            auto device   = VuDevice{};
            ctx::vuDevice = &device;
            std::cout << "Scene init" << std::endl;
            vuRenderer.init(pipelineCache);
            ctx::vuRenderer = &vuRenderer;

            VuMesh jetMesh{};
            VuAssetLoader::LoadGltf(jetPath, jetMesh);

            VuHandle<VuTexture> jetBaseColorTexture;
            jetBaseColorTexture.createHandle()->init({"assets/gltf/jet/textures/texture_baseColor.png"});
            VuResourceManager::writeSampledImageToGlobalPool(jetBaseColorTexture.index, jetBaseColorTexture.get()->imageView);

            VuHandle<VuTexture> jetNormalTexture;
            jetNormalTexture.createHandle()->init({"assets/gltf/jet/textures/texture_normal.png"});
            VuResourceManager::writeSampledImageToGlobalPool(jetNormalTexture.index, jetNormalTexture.get()->imageView);

            VuMesh mountainMesh{};
            VuAssetLoader::LoadGltf(mountainPath, mountainMesh);

            VuHandle<VuTexture> mountainBaseColorTexture;
            mountainBaseColorTexture.createHandle()->init({"assets/gltf/mountain/textures/texture_baseColor.png"});
            VuResourceManager::writeSampledImageToGlobalPool(mountainBaseColorTexture.index, mountainBaseColorTexture.get()->imageView);

            VuHandle<VuTexture> mountainNormalTexture;
            mountainNormalTexture.createHandle()->init({"assets/gltf/mountain/textures/texture_normal.png"});
            VuResourceManager::writeSampledImageToGlobalPool(mountainNormalTexture.index, mountainNormalTexture.get()->imageView);

            pbrShader.initAsGraphicsShader(
                {
                    vuRenderer.pipelineCache,
                    vuRenderer.swapChain.renderPass.renderPass
                }
            );

            uint32                jetMaterial = pbrShader.createMaterial();
            GPU_PBR_MaterialData* jetMatData  = pbrShader.materials[jetMaterial].getPbrMaterialData();
            jetMatData->baseColorTexture      = jetBaseColorTexture.index;
            jetMatData->normalTexture         = jetNormalTexture.index;
            jetMatData->baseColorMul          = {1, 1, 1};


            uint32                mountainMaterial = pbrShader.createMaterial();
            GPU_PBR_MaterialData* mountainMatData  = pbrShader.materials[mountainMaterial].getPbrMaterialData();
            mountainMatData->baseColorTexture      = mountainBaseColorTexture.index;
            mountainMatData->normalTexture         = mountainNormalTexture.index;
            mountainMatData->baseColorMul          = {0.2F, 1, 0.2F};

            prevTime = std::chrono::high_resolution_clock::now();
            while (!vuRenderer.shouldWindowClose()) {
                ctx::PreUpdate();
                ctx::UpdateInput();

                std::chrono::duration<double> deltaTime = std::chrono::high_resolution_clock::now() - prevTime;
                prevTime                                = std::chrono::high_resolution_clock::now();

                mountainTransform.Position.z -= 10.0F * deltaTime.count();

                updateFrameConstant();
                vuRenderer.beginFrame();
                renderMesh(jetMesh, pbrShader.materials[jetMaterial], jetTransform);
                renderMesh(mountainMesh, pbrShader.materials[mountainMaterial], mountainTransform);
                vuRenderer.endFrame();
            }


            vuRenderer.waitIdle();
            jetMesh.uninit();
            vuRenderer.uninit();
        }
    };
}
