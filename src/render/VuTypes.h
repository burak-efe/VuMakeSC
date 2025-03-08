#pragma once

#include <optional>
#include <vector>

#include "Common.h"

namespace Vu {

    struct GPU_Mesh {
        uint32 vertexBufferHandle;
        uint32 vertexCount;
        uint32 meshFlags;
    };

    struct GPU_PBR_MaterialData {

        uint32_t baseColorTexture;
        uint32_t normalTexture;
        float3   baseColorMul;
        uint32_t padding[11];
    };

    struct GPU_PushConstant {
        float4x4 trs;
        uint32   materialBlockIndex;
        GPU_Mesh mesh;
    };

    struct GPU_FrameConst {
        float4x4 view;
        float4x4 proj;
        float4   cameraPos;
        float4   cameraDir;
        float    time;
        float    debugIndex;
    };


    struct VuDisposeStack {
        std::stack<std::function<void()> > disposeStack;

        void push(const std::function<void()>& func) {
            disposeStack.push(func);
        }

        void disposeAll() {

            while (!disposeStack.empty()) {
                std::function<void()> disposeFunc = disposeStack.top();
                disposeFunc();
                disposeStack.pop();
            }
        }
    };

    struct QueueFamilyIndices {
        std::optional<uint32> graphicsFamily;
        std::optional<uint32> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }

        static QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface) {
            //Logic to find graphics queue family
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
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;

        static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface) {
            SwapChainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

            uint32 formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
            }

            uint32 presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }
    };


}
