#pragma once

#include <functional>
#include <stack>
#include "Common.h"
#include "VuMesh.h"
#include "VuSwapChain.h"
#include "VuBuffer.h"
#include "VuMaterial.h"
#include "VuSampler.h"
#include "VuTexture.h"
#include "VuResourceManager.h"

namespace Vu {
    //constexpr uint32 WIDTH = 960;
    //constexpr uint32 HEIGHT = 540;

    struct VuRenderer {
    public:
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore>     imageAvailableSemaphores;
        std::vector<VkSemaphore>     renderFinishedSemaphores;
        std::vector<VkFence>         inFlightFences;
        std::vector<VuBuffer>        uniformBuffers;

        VkSurfaceKHR surface;
        VuSwapChain  swapChain;
        //ImGui_ImplVulkanH_Window imguiMainWindowData;

        uint32 currentFrame           = 0;
        uint32 currentFrameImageIndex = 0;

        VuHandle<VuTexture> debugTexture0;
        VuHandle<VuTexture> debugTexture1;
        VuHandle<VuSampler> debugSampler;

        std::stack<std::function<void()> > disposeStack;

        VkPipelineCache pipelineCache;

        void init(std::vector<char>&         pipelineCache,
                  VkPhysicalDeviceFeatures2& physicalDeviceFeatures2WithChain,
                  VkPipelineCacheCreateInfo& pipelineCacheCreateInfo);

        void uninit();

        bool shouldWindowClose();

        void waitIdle();

        void beginFrame();

        void endFrame();

        void bindMesh(VuMesh& mesh);

        void bindMaterial(const VuMaterial& material);

        void pushConstants(const GPU_PushConstant& pushConstant);

        void drawIndexed(uint32 indexCount);

        void updateFrameConstantBuffer(GPU_FrameConst ubo);

    private:
        void waitForFences();

        void beginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void endRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void initVulkanDevice(std::vector<char>& pipelineCacheBlob, VkPhysicalDeviceFeatures2& physicalDeviceFeaturesWithChain);

        void initVulkanInstance();

        void initSurface();

        void initSwapchain();

        void initCommandBuffers();

        void initSyncObjects();

        void initUniformBuffers();

        void bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer);

    };
}
