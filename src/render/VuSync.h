#pragma once

#include "Common.h"

namespace VuSync {

    inline VkImageMemoryBarrier ImageMemoryBarrier() {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageMemoryBarrier;
    }

    inline void InsertImageMemoryBarrier(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange) {

        VkImageMemoryBarrier imageMemoryBarrier = ImageMemoryBarrier();
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier
        );
    }

    //
    // inline void InsertImageMemoryBarrier2(
    //     VkCommandBuffer cmdbuffer,
    //     VkImage image,
    //     VkAccessFlags2 srcAccessMask,
    //     VkAccessFlags2 dstAccessMask,
    //     VkImageLayout oldImageLayout,
    //     VkImageLayout newImageLayout,
    //     VkPipelineStageFlags2 srcStageMask,
    //     VkPipelineStageFlags2 dstStageMask,
    //     VkImageSubresourceRange subresourceRange
    //
    // ) {
    //
    //     VkImageMemoryBarrier2 imageMemoryBarrier2{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = srcStageMask,
    //         .srcAccessMask = srcAccessMask,
    //         .dstStageMask = dstStageMask,
    //         .dstAccessMask = dstAccessMask,
    //         .oldLayout = oldImageLayout,
    //         .newLayout = newImageLayout,
    //         .srcQueueFamilyIndex = 0,
    //         .dstQueueFamilyIndex = 0,
    //         .image = image,
    //         .subresourceRange = subresourceRange
    //     };
    //
    //     VkDependencyFlags dependencyFlags = 0;
    //
    //     VkDependencyInfo dependInfo{
    //         .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //         .pNext = nullptr,
    //         .dependencyFlags = dependencyFlags,
    //         .memoryBarrierCount = 0,
    //         .pMemoryBarriers = nullptr,
    //         .bufferMemoryBarrierCount = 0,
    //         .pBufferMemoryBarriers = nullptr,
    //         .imageMemoryBarrierCount = 1,
    //         .pImageMemoryBarriers = &imageMemoryBarrier2,
    //     };
    //     vkCmdPipelineBarrier2(cmdbuffer, &dependInfo);
    // }
}
