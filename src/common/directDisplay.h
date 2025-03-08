// Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#pragma once

#include <vector>
#include "vulkan/vulkan_sc.h"

VkResult createDirectSurface(VkPhysicalDevice physdev,
                             VkInstance       instance,
                             int              width,
                             int              height,
                             VkSurfaceKHR&    surface);

VkResult createSwapchain(VkPhysicalDevice physdev,
                         VkDevice         dev,
                         VkSurfaceKHR     surface,
                         uint32_t         desired_image_count,
                         VkFormat         colorFormat,
                         uint32_t         width,
                         uint32_t         height,
                         VkSwapchainKHR&  swapchain);

VkResult buildPresentCommandBuffers(std::vector<VkImage>&         images,
                                    std::vector<VkCommandBuffer>& postPresentCmdBuffers,
                                    std::vector<VkCommandBuffer>& prePresentCmdBuffers);

VkResult prepareNextImage(VkDevice                            dev,
                          VkQueue                             queue,
                          VkSwapchainKHR                      swapchain,
                          VkSemaphore                         presentCompleteSem,
                          const std::vector<VkCommandBuffer>& postPresentCmdBuffers,
                          uint32_t&                           imageIndex);

VkResult presentImage(VkQueue                      queue,
                      VkSwapchainKHR               swapchain,
                      VkSemaphore                  renderCompleteSem,
                      std::vector<VkCommandBuffer> prePresentCmdBuffers,
                      uint32_t                     imageIndex);
