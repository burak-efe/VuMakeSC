#pragma once
#include "Common.h"
#include "VuDepthStencil.h"
#include "VuRenderPass.h"
#include "VuTypes.h"

namespace Vu {
    struct VuSwapChain {
    public:
        VkSwapchainKHR swapChain;
        VuRenderPass renderPass;
        VuDepthStencil depthStencil;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> framebuffers;

        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        uint32_t imageCount;
        uint32_t queueNodeIndex = UINT32_MAX;

        void init(VkSurfaceKHR surface);

        void uninit();

        //void resetSwapChain(VkSurfaceKHR surface);

        void beginRenderPass(VkCommandBuffer commandBuffer, uint32 frameIndex);

        void endRenderPass(VkCommandBuffer commandBuffer);

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32 width, uint32 height);

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    private:
        void createSwapChain(VkSurfaceKHR surfaceKHR);

        void createImageViews(VkDevice device);

        void createFramebuffers();
    };
}
