#include <filesystem>
#include "VuRenderer.h"
#include "VuResourceManager.h"
#include "VuShader.h"
#include "directDisplay.h"


namespace Vu {
    void VuRenderer::init(std::vector<char>& pipelineCache) {
        initVulkanInstance();
        ctx::vuDevice->initPhysicalDevice();
        initSurface();
        initVulkanDevice(pipelineCache);
        initSwapchain();

        VuBuffer::initGlobalStagingBuffer({
            64U * 1024U * 1024U,
            1U,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            0U
        });

        ctx::vuDevice->initBindless(config::BINDLESS_CONFIG_INFO, config::MAX_FRAMES_IN_FLIGHT);

        VuResourceManager::init(config::BINDLESS_CONFIG_INFO);

        disposeStack.push([&] { VuResourceManager::uninit(); });

        VuMaterialDataPool::init();
        disposeStack.push([&] { VuMaterialDataPool::uninit(); });


        initUniformBuffers();
        initCommandBuffers();
        initSyncObjects();


        //debug resources
        debugTexture0.createHandle()->init({std::filesystem::path("assets/textures/error.png"), VK_FORMAT_R8G8B8A8_UNORM});
        VuResourceManager::writeSampledImageToGlobalPool(debugTexture0.index, debugTexture0.get()->imageView);
        disposeStack.push([this] { auto noop = debugTexture0.destroyHandle(); });

        debugTexture1.createHandle()->init({std::filesystem::path("assets/textures/debug_n.png"), VK_FORMAT_R8G8B8A8_UNORM});
        VuResourceManager::writeSampledImageToGlobalPool(debugTexture1.index, debugTexture1.get()->imageView);
        disposeStack.push([this] { auto noop = debugTexture1.destroyHandle(); });

        debugSampler.createHandle()->init({});
        VuResourceManager::writeSamplerToGlobalPool(debugSampler.index, debugSampler.get()->vkSampler);
        disposeStack.push([this] { auto noop = debugSampler.destroyHandle(); });
    }


    void VuRenderer::initVulkanInstance() {
        ctx::vuDevice->initInstance(config::ENABLE_VALIDATION_LAYERS_LAYERS, config::VALIDATION_LAYERS, config::INSTANCE_EXTENSIONS);
    }

    void VuRenderer::initVulkanDevice(std::vector<char>& pipelineCacheBlob) {


        VkPipelineCacheCreateInfo pipelineCacheCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_PIPELINE_CACHE_CREATE_READ_ONLY_BIT | VK_PIPELINE_CACHE_CREATE_USE_APPLICATION_STORAGE_BIT,
            .initialDataSize = pipelineCacheBlob.size(),
            .pInitialData = pipelineCacheBlob.data()
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


        ctx::vuDevice->initDevice({
            config::ENABLE_VALIDATION_LAYERS_LAYERS,
            deviceFeatures2,
            surface,
            config::DEVICE_EXTENSIONS
        });

        vkCreatePipelineCache(ctx::vuDevice->device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
    }

    void VuRenderer::initSurface() {


        VkCheck(createDirectSurface(ctx::vuDevice->physicalDevice, ctx::vuDevice->instance, config::SCREEN_WIDTH, config::SCREEN_HEIGHT,
                                    surface));
    }

    void VuRenderer::initSwapchain() {
        swapChain = VuSwapChain{};
        swapChain.init(surface);
        disposeStack.push([&] { swapChain.uninit(); });
    }

    void VuRenderer::initCommandBuffers() {
        commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = ctx::vuDevice->commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
        VkCheck(vkAllocateCommandBuffers(ctx::vuDevice->device, &allocInfo, commandBuffers.data()));
    }

    void VuRenderer::initSyncObjects() {
        imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkCheck(vkCreateSemaphore(ctx::vuDevice->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            VkCheck(vkCreateSemaphore(ctx::vuDevice->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
            VkCheck(vkCreateFence(ctx::vuDevice->device, &fenceInfo, nullptr, &inFlightFences[i]));
        }

        disposeStack.push([vr = *this] {
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {

                vkDestroySemaphore(ctx::vuDevice->device, vr.imageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(ctx::vuDevice->device, vr.renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(ctx::vuDevice->device, vr.inFlightFences[i], nullptr);
            }
        });

    }

    void VuRenderer::initUniformBuffers() {
        uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDeviceSize bufferSize = sizeof(GPU_FrameConst);

            uniformBuffers[i] = VuBuffer();
            uniformBuffers[i].init({
                .length = 1,
                .strideInBytes = bufferSize,
                .usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                .memoryPropertyFlags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                .createFlags = 0
            });
            uniformBuffers[i].map();
        }
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VuResourceManager::writeUBO_ToGlobalPool(0, i, uniformBuffers[i]);
        }


        disposeStack.push([vr = *this] {
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
                auto v = vr.uniformBuffers;
                v[i].uninit();
            }
        });
    }

    void VuRenderer::bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer) {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            ctx::vuDevice->globalPipelineLayout,
            0,
            1,
            &ctx::vuDevice->globalDescriptorSets[currentFrame],
            0,
            nullptr
        );
    }

    void VuRenderer::uninit() {
        vkDeviceWaitIdle(ctx::vuDevice->device);
        while (!disposeStack.empty()) {
            std::function<void()> disposeFunc = disposeStack.top();
            disposeFunc();
            disposeStack.pop();
        }
    }
}
