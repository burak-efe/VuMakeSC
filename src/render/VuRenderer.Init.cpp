#include <filesystem>
#include "VuRenderer.h"
#include "VuResourceManager.h"
#include "VuShader.h"
#include "directDisplay.h"


namespace Vu {
    void VuRenderer::init(std::vector<char>&         pipelineCacheBinary,
                          VkPhysicalDeviceFeatures2& physicalDeviceFeatures2WithChain,
                          VkPipelineCacheCreateInfo& pipelineCacheCreateInfo) {
        initVulkanInstance();
        ctx::vuDevice->initPhysicalDevice();
        initSurface();
        initVulkanDevice(pipelineCacheBinary, physicalDeviceFeatures2WithChain);

        VkCheck(vkCreatePipelineCache(ctx::vuDevice->device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

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

    void VuRenderer::initVulkanDevice(std::vector<char>& pipelineCacheBlob, VkPhysicalDeviceFeatures2& physicalDeviceFeaturesWithChain) {


        ctx::vuDevice->initDevice({
            config::ENABLE_VALIDATION_LAYERS_LAYERS,
            physicalDeviceFeaturesWithChain,
            surface,
            config::DEVICE_EXTENSIONS
        });


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
