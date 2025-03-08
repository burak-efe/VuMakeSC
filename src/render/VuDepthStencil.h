#pragma once

#include "Common.h"
#include "VuUtils.h"
#include "VuCtx.h"
#include "VuDevice.h"
#include "VuImage.h"

namespace Vu {
    struct VuDepthStencil {
        VkDeviceMemory memory;
        VkImage        image;
        VkImageView    imageView;
        VkFormat       depthFormat;

        void init(VkExtent2D extent2D, VkFormat format = VK_FORMAT_D32_SFLOAT_S8_UINT) {
            VkExtent3D depthImageExtent = {
                extent2D.width,
                extent2D.height,
                1U
            };

            depthFormat                       = format;

            VuImage::createImage(extent2D.width,
                                 extent2D.height,
                                 depthFormat,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 image,
                                 memory);

            VuImage::createImageView(depthFormat, image,VK_IMAGE_ASPECT_DEPTH_BIT, imageView);

        }

        void uninit() {
            vkDestroyImageView(ctx::vuDevice->device, imageView, nullptr);
        }

        static VkPipelineDepthStencilStateCreateInfo fillDepthStencilCreateInfo(bool        bDepthTest,
                                                                                bool        bDepthWrite,
                                                                                VkCompareOp compareOp) {
            VkPipelineDepthStencilStateCreateInfo info = {};
            info.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            info.depthTestEnable                       = bDepthTest ? VK_TRUE : VK_FALSE;
            info.depthWriteEnable                      = bDepthWrite ? VK_TRUE : VK_FALSE;
            info.depthCompareOp                        = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
            info.depthBoundsTestEnable                 = VK_FALSE;
            return info;
        }

    };
}
