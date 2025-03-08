/*
 * Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "directDisplay.h"
#include <cassert>
#include <iostream>


static VkDisplayKHR g_display;

bool operator==(const VkExtent2D& l, const VkExtent2D& r) {
    return l.width == r.width && l.height == r.height;
}

bool operator!=(const VkExtent2D& l, const VkExtent2D& r) {
    return !(l == r);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkResult createDirectSurface(const VkPhysicalDevice physdev,
                             const VkInstance       instance,
                             int                   width,
                             int                   height,
                             VkSurfaceKHR&          surface) {
    VkResult                                 err;
    uint32_t                                 displayCount = 0;
    uint32_t                                 planeCount   = 0;
    std::vector<VkDisplayPropertiesKHR>      displayProps;
    std::vector<VkDisplayPlanePropertiesKHR> planeProps;
    VkDisplayModeKHR                         mode      = VK_NULL_HANDLE;
    uint32_t                                 bestPlane = UINT32_MAX;
    VkDisplayPlaneAlphaFlagBitsKHR           alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;


    // Get a list of displays on the physical device
    vkGetPhysicalDeviceDisplayPropertiesKHR(physdev, &displayCount, nullptr);

    displayProps.resize(displayCount);
    vkGetPhysicalDeviceDisplayPropertiesKHR(physdev, &displayCount, displayProps.data());

    // Get a list of display planes on the physical device
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physdev, &planeCount, nullptr);

    planeProps.resize(planeCount);
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physdev, &planeCount, planeProps.data());

    // Get the list of display modes each display supports
    for (uint32_t i = 0; i < displayCount; ++i) {
        g_display          = displayProps[i].display;
        uint32_t modeCount = 0;


        vkGetDisplayModePropertiesKHR(physdev, g_display, &modeCount, nullptr);

        if (modeCount == 0) {

            continue;
        }

        std::vector<VkDisplayModePropertiesKHR> modeProps(modeCount);
        vkGetDisplayModePropertiesKHR(physdev, g_display, &modeCount, modeProps.data());

        // Find a mode with resolution matching the monitor's native resolution. Prefer higher
        // refresh rates.
        VkDisplayModePropertiesKHR bestModeProp = {};
        for (uint32_t j = 0; j < modeProps.size(); ++j) {
            if (modeProps[j].parameters.visibleRegion != displayProps[i].physicalResolution) {
                continue;
            }

            if (bestModeProp.parameters.refreshRate < modeProps[j].parameters.refreshRate) {
                bestModeProp = modeProps[j];
            }
        }

        mode   = bestModeProp.displayMode;
        width  = bestModeProp.parameters.visibleRegion.width;
        height = bestModeProp.parameters.visibleRegion.height;

        // Find a plane that matches these criteria, in order of preference:
        // -Is compatible with the chosen display + mode.
        // -Isn't currently bound to another display.
        // -Supports per-pixel alpha, if possible.
        for (uint32_t j = 0; j < planeCount; ++j) {
            uint32_t supportedDisplayCount = 0;
            vkGetDisplayPlaneSupportedDisplaysKHR(physdev, j, &supportedDisplayCount, nullptr);

            if (supportedDisplayCount == 0) {
                continue;
            }

            std::vector<VkDisplayKHR> supportedDisplays(supportedDisplayCount);
            vkGetDisplayPlaneSupportedDisplaysKHR(physdev,j,&supportedDisplayCount,supportedDisplays.data());

            bool planeSupportsDevice = false;
            for (uint32_t k = 0; k < supportedDisplayCount; ++k) {
                if (supportedDisplays[k] == g_display) {
                    // If no supported plane has yet been found, this is
                    // currently the best available plane.
                    if (bestPlane == UINT32_MAX)
                        bestPlane = j;
                    planeSupportsDevice = true;
                }
            }

            // If the plane can't be used with the chosen display, keep looking.
            // Each display must have at least one compatible plane.
            if (!planeSupportsDevice) {
                continue;
            }

            // If the plane passed the above test and is currently bound to the
            // desired display, or is not in use, it is the best plane found so
            // far.
            if ((planeProps[j].currentDisplay == VK_NULL_HANDLE) || (planeProps[j].currentDisplay == g_display)) {
                bestPlane = j;
            } else {
                continue;
            }

            VkDisplayPlaneCapabilitiesKHR planeCaps;
            vkGetDisplayPlaneCapabilitiesKHR(physdev, mode, j, &planeCaps);

            // Prefer a plane that supports per-pixel alpha.
            if (planeCaps.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR) {
                // This is the perfect plane for the given criteria.  Use it.
                bestPlane = j;
                alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
                break;
            }
        }

        // We've found a satisfactory display and mode
        if (g_display != VK_NULL_HANDLE && mode != VK_NULL_HANDLE) {
            break;
        }

    }

    if (g_display == VK_NULL_HANDLE || mode == VK_NULL_HANDLE) {
        // No suitable display + mode could be found.  Abort.
        std::cout << "ERR: No suitable display could be found for --direct-to-display.\n";
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Success.  Create a VkSurfaceKHR object for this plane.
    const VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo = {
        VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,             // sType
        nullptr,                                                       // pNext
        0,                                                             // flags
        mode,                                                          // displayMode
        bestPlane,                                                     // planeIndex
        planeProps[bestPlane].currentStackIndex,                       // planeStackIndex
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,                         // transform
        1.0f,                                                          // globalAlpha
        alphaMode,                                                     // alphaMode
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, // presentable image size
    };

    err = vkCreateDisplayPlaneSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);


    return err;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VkResult createSwapchain(const VkPhysicalDevice physdev,
                         const VkDevice         dev,
                         const VkSurfaceKHR     surface,
                         const uint32_t         desiredImageCount,
                         const VkFormat         colorFormat,
                         const uint32_t         width,
                         const uint32_t         height,
                         VkSwapchainKHR&        swapchain) {
    // Get list of supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physdev, surface, &formatCount, surfaceFormats.data());

    // Check if the surface supports the specified format
    bool surfaceFormatFound = false;
    for (uint32_t i = 0; i < formatCount; ++i) {
        if (surfaceFormats[i].format == colorFormat) {
            surfaceFormatFound = true;
        }
    }
    assert(surfaceFormatFound);

    // Get physical device surface properties
    VkSurfaceCapabilitiesKHR surfCaps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physdev, surface, &surfCaps);

    // Check if the surface supports the specified extent
    assert(width >= surfCaps.minImageExtent.width && width <= surfCaps.maxImageExtent.width);
    assert(height >= surfCaps.minImageExtent.height && height <= surfCaps.maxImageExtent.height);

    // Get available present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physdev, surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physdev,
                                              surface,
                                              &presentModeCount,
                                              presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (size_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            break;
        }
    }

    assert(desiredImageCount >= surfCaps.minImageCount);
    assert(desiredImageCount <= surfCaps.maxImageCount);

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCaps.currentTransform;
    }

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext                    = nullptr;
    swapchainInfo.surface                  = surface;
    swapchainInfo.minImageCount            = desiredImageCount;
    swapchainInfo.imageFormat              = colorFormat;
    swapchainInfo.imageColorSpace          = surfaceFormats[0].colorSpace;
    swapchainInfo.imageExtent              = {(uint32_t) width, (uint32_t) height};
    swapchainInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                               | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapchainInfo.preTransform          = (VkSurfaceTransformFlagBitsKHR) preTransform;
    swapchainInfo.imageArrayLayers      = 1;
    swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.queueFamilyIndexCount = 0;
    swapchainInfo.pQueueFamilyIndices   = nullptr;
    swapchainInfo.presentMode           = presentMode;
    swapchainInfo.clipped               = true;
    swapchainInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.oldSwapchain          = VK_NULL_HANDLE;

    VkResult err = vkCreateSwapchainKHR(dev, &swapchainInfo, nullptr, &swapchain);

    return err;
}

VkResult buildPresentCommandBuffers(std::vector<VkImage>&         images,
                                    std::vector<VkCommandBuffer>& postPresentCmdBuffers,
                                    std::vector<VkCommandBuffer>& prePresentCmdBuffers) {
    VkResult                 err;
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkImageMemoryBarrier postPresentBarrier = {};
    postPresentBarrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    postPresentBarrier.srcAccessMask        = 0;
    postPresentBarrier.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    postPresentBarrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    postPresentBarrier.newLayout            = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    postPresentBarrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    postPresentBarrier.subresourceRange     = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageMemoryBarrier prePresentBarrier = {};
    prePresentBarrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    prePresentBarrier.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    prePresentBarrier.dstAccessMask        = VK_ACCESS_MEMORY_READ_BIT;
    prePresentBarrier.oldLayout            = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    prePresentBarrier.newLayout            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    prePresentBarrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.subresourceRange     = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    for (uint32_t i = 0; i < postPresentCmdBuffers.size(); ++i) {
        // Command buffer for post present barrier

        // Insert a post present image barrier to transform the image back to a
        // color attachment that our render pass can write to
        // We always use undefined image layout as the source as it doesn't actually matter
        // what is done with the previous image contents

        err = vkBeginCommandBuffer(postPresentCmdBuffers[i], &cmdBufInfo);


        postPresentBarrier.image = images[i];

        vkCmdPipelineBarrier(postPresentCmdBuffers[i],
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &postPresentBarrier);

        err = vkEndCommandBuffer(postPresentCmdBuffers[i]);


        // Command buffers for pre present barrier

        // Submit a pre present image barrier to the queue
        // Transforms the (framebuffer) image layout from color attachment to present(khr) for presenting to the swapchain

        err = vkBeginCommandBuffer(prePresentCmdBuffers[i], &cmdBufInfo);


        prePresentBarrier.image = images[i];

        vkCmdPipelineBarrier(prePresentCmdBuffers[i],
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                             0,
                             0,
                             nullptr, // No memory barriers,
                             0,
                             nullptr, // No buffer barriers,
                             1,
                             &prePresentBarrier);

        err = vkEndCommandBuffer(prePresentCmdBuffers[i]);

    }

    return err;
}

VkResult prepareNextImage(const VkDevice                      dev,
                          const VkQueue                       queue,
                          const VkSwapchainKHR                swapchain,
                          const VkSemaphore                   presentCompleteSem,
                          const std::vector<VkCommandBuffer>& postPresentCmdBuffers,
                          uint32_t&                           imageIndex) {
    VkResult             err;
    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask  = &pipelineStages;
    submitInfo.commandBufferCount = 1;

    // Get next image in the swapchain
    err = vkAcquireNextImageKHR(dev,
                                swapchain,
                                UINT64_MAX,
                                presentCompleteSem,
                                VK_NULL_HANDLE,
                                &imageIndex);

    assert(err == VK_SUCCESS);

    // Submit the post present image barrier to transform the image back to a color attachment
    // that can be used to write to by our render pass
    submitInfo.pCommandBuffers = &postPresentCmdBuffers[imageIndex];

    err = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    assert(err == VK_SUCCESS);

    err = vkQueueWaitIdle(queue);
    assert(err == VK_SUCCESS);

    return err;
}

VkResult presentImage(const VkQueue                      queue,
                      const VkSwapchainKHR               swapchain,
                      const VkSemaphore                  renderCompleteSem,
                      const std::vector<VkCommandBuffer> prePresentCmdBuffers,
                      const uint32_t                     imageIndex) {
    VkResult             err;
    VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    // Submit the pre present image barrier to transform the image back to a layout
    // for presenting.
    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask  = &pipelineStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &prePresentCmdBuffers[imageIndex];
    err                           = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    assert(err == VK_SUCCESS);

    // Present the current buffer to the swapchain
    // We pass the signal semaphore from the submit info
    // to ensure that the image is not rendered until
    // all commands have been submitted
    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapchain;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pWaitSemaphores    = &renderCompleteSem;
    presentInfo.waitSemaphoreCount = 1;
    err                            = vkQueuePresentKHR(queue, &presentInfo);
    assert(err == VK_SUCCESS);

    return err;
}
