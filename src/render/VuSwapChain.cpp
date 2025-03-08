#include "VuSwapChain.h"

#include <algorithm>

namespace Vu {

    void VuSwapChain::init(VkSurfaceKHR surface) {
        createSwapChain(surface);
        createImageViews(ctx::vuDevice->device);
        depthStencil.init(swapChainExtent);
        renderPass.Init(swapChainImageFormat, depthStencil.depthFormat);
        createFramebuffers();
    }

    void VuSwapChain::uninit() {
        for (auto imageView: swapChainImageViews) {
            vkDestroyImageView(ctx::vuDevice->device, imageView, nullptr);
        }
        for (auto framebuffer: framebuffers) {
            vkDestroyFramebuffer(ctx::vuDevice->device, framebuffer, nullptr);
        }

        renderPass.uninit();
        depthStencil.uninit();
        //vkDestroySwapchainKHR(ctx::vuDevice->device, swapChain, nullptr);
    }


    VkSurfaceFormatKHR VuSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VuSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VuSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32 width, uint32 height) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max()) {
            return capabilities.currentExtent;
        }
        //int width, height;
        //SDL_GetWindowSize(ctx::window, &width, &height);
        std::cout << "width: " << width << std::endl;
        VkExtent2D actualExtent = {
            static_cast<uint32>(width),
            static_cast<uint32>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,capabilities.maxImageExtent.height);
        return actualExtent;
    }

    QueueFamilyIndices VuSwapChain::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;

        for (const auto& queuefamily: queueFamilies) {
            if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }

    // void VuSwapChain::resetSwapChain(VkSurfaceKHR surface) {
    //     for (auto imageView: swapChainImageViews) {
    //         vkDestroyImageView(ctx::vuDevice->device, imageView, nullptr);
    //     }
    //     for (auto framebuffer: framebuffers) {
    //         vkDestroyFramebuffer(ctx::vuDevice->device, framebuffer, nullptr);
    //     }
    //     depthStencil.uninit();
    //     vkDestroySwapchainKHR(ctx::vuDevice->device, swapChain, nullptr);
    //     createSwapChain(surface);
    //     createImageViews(ctx::vuDevice->device);
    //     depthStencil.init(swapChainExtent);
    //     createFramebuffers();
    // }

    void VuSwapChain::beginRenderPass(VkCommandBuffer commandBuffer, uint32 frameIndex) {

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.02f, 0.02f, 0.02f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass.renderPass;
        renderPassInfo.framebuffer = framebuffers[frameIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VuSwapChain::endRenderPass(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VuSwapChain::createSwapChain(VkSurfaceKHR surfaceKHR) {
        SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(ctx::vuDevice->physicalDevice, surfaceKHR);
        VkExtent2D extend = chooseSwapExtent(swapChainSupport.capabilities, config::SCREEN_WIDTH, config::SCREEN_HEIGHT);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0
            && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surfaceKHR;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extend;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(ctx::vuDevice->physicalDevice, surfaceKHR);
        uint32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        //
        {
            VkCheck(vkCreateSwapchainKHR(ctx::vuDevice->device, &createInfo, nullptr, &swapChain));
        }

        //
        {
            vkGetSwapchainImagesKHR(ctx::vuDevice->device, swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(ctx::vuDevice->device, swapChain, &imageCount, swapChainImages.data());
        }

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extend;
    }

    void VuSwapChain::createImageViews(VkDevice device) {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;


            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VkCheck(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]));
        }
    }

    void VuSwapChain::createFramebuffers() {
        framebuffers.resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array attachments = {
                swapChainImageViews[i],
                depthStencil.imageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            VkCheck(vkCreateFramebuffer(ctx::vuDevice->device, &framebufferInfo, nullptr, &framebuffers[i]));
        }
    }
}