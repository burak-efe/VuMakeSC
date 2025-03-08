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
            std::vector<char> pipelineCacheBinary = readFile("assets\\shaders\\pipeline_cache.bin");
            auto              device        = VuDevice{};
            ctx::vuDevice                   = &device;

            VkPipelineCacheCreateInfo pipelineCacheCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_PIPELINE_CACHE_CREATE_READ_ONLY_BIT | VK_PIPELINE_CACHE_CREATE_USE_APPLICATION_STORAGE_BIT,
                .initialDataSize = pipelineCacheBinary.size(),
                .pInitialData = pipelineCacheBinary.data()
            };

            VkPipelinePoolSize poolSize{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_POOL_SIZE,
                .pNext = nullptr,
                .poolEntrySize = 8U * 1024U * 1024U,
                .poolEntryCount = 8U
            };

            VkDeviceObjectReservationCreateInfo scReservationCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_OBJECT_RESERVATION_CREATE_INFO,
                .pNext = nullptr,
                .pipelineCacheCreateInfoCount = 1U,
                .pPipelineCacheCreateInfos = &pipelineCacheCreateInfo,
                .pipelinePoolSizeCount = 1U,
                .pPipelinePoolSizes = &poolSize,
                .semaphoreRequestCount = 32U,
                .commandBufferRequestCount = 32U,
                .fenceRequestCount = 32U,
                .deviceMemoryRequestCount = 4096U,
                .bufferRequestCount = 4096U,
                .imageRequestCount = 4096U,
                .eventRequestCount = 32U,
                .queryPoolRequestCount = 32U,
                .bufferViewRequestCount = 4096U,
                .imageViewRequestCount = 4096U,
                .layeredImageViewRequestCount = 32U,
                .pipelineCacheRequestCount = 32U,
                .pipelineLayoutRequestCount = 32U,
                .renderPassRequestCount = 32U,
                .graphicsPipelineRequestCount = 32U,
                .computePipelineRequestCount = 32U,
                .descriptorSetLayoutRequestCount = 32U,
                .samplerRequestCount = 32U,
                .descriptorPoolRequestCount = 32U,
                .descriptorSetRequestCount = 32U,
                .framebufferRequestCount = 32U,
                .commandPoolRequestCount = 32U,
                .samplerYcbcrConversionRequestCount = 0U,
                .surfaceRequestCount = 32U,
                .swapchainRequestCount = 32U,
                .displayModeRequestCount = 32U,
                .subpassDescriptionRequestCount = 32U,
                .attachmentDescriptionRequestCount = 32U,
                .descriptorSetLayoutBindingRequestCount = 32U,
                .descriptorSetLayoutBindingLimit = 32U,
                .maxImageViewMipLevels = 8U,
                .maxImageViewArrayLayers = 8U,
                .maxLayeredImageViewMipLevels = 8U,
                .maxOcclusionQueriesPerPool = 32U,
                .maxPipelineStatisticsQueriesPerPool = 32U,
                .maxTimestampQueriesPerPool = 32U,
                .maxImmutableSamplersPerDescriptorSetLayout = 32U,
            };

            VkPhysicalDeviceVulkanSC10Features sc10Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_SC_1_0_FEATURES,
                .pNext = &scReservationCreateInfo,
                .shaderAtomicInstructions = VK_FALSE
            };

            VkPhysicalDeviceSynchronization2Features synchronization2Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
                .pNext = &sc10Features,
                .synchronization2 = VK_TRUE
            };

            VkPhysicalDeviceVulkan12Features vkPhysicalDeviceVulkan12Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = &synchronization2Features,
                .samplerMirrorClampToEdge = VK_FALSE,
                .drawIndirectCount = VK_FALSE,
                .storageBuffer8BitAccess = VK_FALSE,
                .uniformAndStorageBuffer8BitAccess = VK_FALSE,
                .storagePushConstant8 = VK_FALSE,
                .shaderBufferInt64Atomics = VK_FALSE,
                .shaderSharedInt64Atomics = VK_FALSE,
                .shaderFloat16 = VK_FALSE,
                .shaderInt8 = VK_FALSE,
                .descriptorIndexing = VK_TRUE,
                .shaderInputAttachmentArrayDynamicIndexing = VK_TRUE,
                .shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE,
                .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
                .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
                .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
                .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
                .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
                .shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE,
                .shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE,
                .shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE,
                .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
                .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
                .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
                .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
                .descriptorBindingPartiallyBound = VK_TRUE,
                .descriptorBindingVariableDescriptorCount = VK_FALSE,
                .runtimeDescriptorArray = VK_TRUE,
                .samplerFilterMinmax = VK_FALSE,
                .scalarBlockLayout = VK_TRUE,
                .imagelessFramebuffer = VK_FALSE,
                .uniformBufferStandardLayout = VK_FALSE,
                .shaderSubgroupExtendedTypes = VK_FALSE,
                .separateDepthStencilLayouts = VK_FALSE,
                .hostQueryReset = VK_FALSE,
                .timelineSemaphore = VK_FALSE,
                .bufferDeviceAddress = VK_TRUE,
                .bufferDeviceAddressCaptureReplay = VK_FALSE,
                .bufferDeviceAddressMultiDevice = VK_FALSE,
                .vulkanMemoryModel = VK_FALSE,
                .vulkanMemoryModelDeviceScope = VK_FALSE,
                .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
                .shaderOutputViewportIndex = VK_FALSE,
                .shaderOutputLayer = VK_FALSE,
                .subgroupBroadcastDynamicId = VK_FALSE,
            };


            VkPhysicalDeviceFeatures2 deviceFeatures2{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vkPhysicalDeviceVulkan12Features,
                .features = {
                    .samplerAnisotropy = VK_TRUE,
                    .shaderInt64 = VK_TRUE
                },
            };


            vuRenderer.init(pipelineCacheBinary,deviceFeatures2,pipelineCacheCreateInfo);
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
