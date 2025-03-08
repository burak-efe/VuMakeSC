#pragma once

#include <vulkan/vulkan_sc.h>

#define VK_RESULT_CASE(value) case value: return #value;

inline const char* VkResultToString(VkResult result) {
    switch (result) {
        VK_RESULT_CASE(VK_SUCCESS)
        VK_RESULT_CASE(VK_NOT_READY)
        VK_RESULT_CASE(VK_TIMEOUT)
        VK_RESULT_CASE(VK_EVENT_SET)
        VK_RESULT_CASE(VK_EVENT_RESET)
        VK_RESULT_CASE(VK_INCOMPLETE)
        VK_RESULT_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
        VK_RESULT_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
        VK_RESULT_CASE(VK_ERROR_INITIALIZATION_FAILED)
        VK_RESULT_CASE(VK_ERROR_DEVICE_LOST)
        VK_RESULT_CASE(VK_ERROR_MEMORY_MAP_FAILED)
        VK_RESULT_CASE(VK_ERROR_LAYER_NOT_PRESENT)
        VK_RESULT_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
        VK_RESULT_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
        VK_RESULT_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
        VK_RESULT_CASE(VK_ERROR_TOO_MANY_OBJECTS)
        VK_RESULT_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
        VK_RESULT_CASE(VK_ERROR_FRAGMENTED_POOL)
        VK_RESULT_CASE(VK_ERROR_UNKNOWN)
        VK_RESULT_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)
        VK_RESULT_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)
        VK_RESULT_CASE(VK_ERROR_FRAGMENTATION)
        VK_RESULT_CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        VK_RESULT_CASE(VK_ERROR_VALIDATION_FAILED)
        VK_RESULT_CASE(VK_ERROR_INVALID_PIPELINE_CACHE_DATA)
        VK_RESULT_CASE(VK_ERROR_NO_PIPELINE_MATCH)
        VK_RESULT_CASE(VK_ERROR_SURFACE_LOST_KHR)
        VK_RESULT_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
        VK_RESULT_CASE(VK_SUBOPTIMAL_KHR)
        VK_RESULT_CASE(VK_ERROR_OUT_OF_DATE_KHR)
        VK_RESULT_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
        VK_RESULT_CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        VK_RESULT_CASE(VK_ERROR_NOT_PERMITTED_KHR)
        VK_RESULT_CASE(VK_RESULT_MAX_ENUM)
        default: return "UNKNOWN_VK_RESULT";
    }
}


inline void PrintAvailableVulkanLayers() {
    uint32_t layerCount = 0;
    VkResult result     = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (result != VK_SUCCESS || layerCount == 0) {
        std::cerr << "Failed to enumerate Vulkan layers or no layers available." << std::endl;
        return;
    }

    std::vector<VkLayerProperties> layers(layerCount);
    result = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to retrieve Vulkan layers." << std::endl;
        return;
    }

    std::cout << "Available Vulkan Layers:" << std::endl;
    for (const auto& layer: layers) {
        std::cout << "- " << layer.layerName << " (" << layer.description << ")" << std::endl;
        std::cout << "Driver Version: "
                << VK_API_VERSION_MAJOR(layer.specVersion) << "."
                << VK_API_VERSION_MINOR(layer.specVersion) << "."
                << VK_API_VERSION_PATCH(layer.specVersion) << std::endl;
    }
}

inline void PrintDriverInfo(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    uint32_t driverVersion = properties.driverVersion;
    uint32_t apiVersion    = properties.apiVersion;

    std::cout << "Driver Name: " << properties.deviceName << std::endl;
    std::cout << "Driver Version: "
            << VK_API_VERSION_MAJOR(driverVersion) << "."
            << VK_API_VERSION_MINOR(driverVersion) << "."
            << VK_API_VERSION_PATCH(driverVersion) << std::endl;
    std::cout << "API Version: "
            << VK_API_VERSION_MAJOR(apiVersion) << "."
            << VK_API_VERSION_MINOR(apiVersion) << "."
            << VK_API_VERSION_PATCH(apiVersion) << std::endl;
}

inline void PrintAvailableInstanceExtensions() {
    uint32_t extensionCount = 0;
    VkResult result         = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS || extensionCount == 0) {
        std::cerr << "Failed to enumerate Vulkan extensions or no extensions available." << std::endl;
        return;
    }

    std::vector<VkExtensionProperties> extensions(extensionCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to retrieve Vulkan extensions." << std::endl;
        return;
    }

    std::cout << "Available Vulkan Extensions:" << std::endl;
    for (const auto& extension: extensions) {
        std::cout << "- " << extension.extensionName << " (spec version: " << extension.specVersion << ")" << std::endl;
    }
}

inline void PrintAvailableDeviceExtensions(VkPhysicalDevice device) {
    uint32_t extensionCount = 0;
    VkResult result         = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    if (result != VK_SUCCESS || extensionCount == 0) {
        std::cerr << "Failed to enumerate device extensions or no extensions available." << std::endl;
        return;
    }

    std::vector<VkExtensionProperties> extensions(extensionCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to retrieve device extensions." << std::endl;
        return;
    }

    std::cout << "Available Device Extensions:" << std::endl;
    for (const auto& extension: extensions) {
        std::cout << "- " << extension.extensionName << " (spec version: " << extension.specVersion << ")" << std::endl;
    }
}


inline void PrintDisplayProperties(VkPhysicalDevice physicalDevice) {
    uint32_t displayCount = 0;

    // Get the number of displays
    VkResult result = vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, nullptr);
    if (result != VK_SUCCESS || displayCount == 0) {
        std::cerr << "No displays found or failed to get display properties.\n";
        return;
    }

    // Allocate space for the display properties
    std::vector<VkDisplayPropertiesKHR> displayProperties(displayCount);

    // Get display properties
    result = vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &displayCount, displayProperties.data());
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to retrieve display properties.\n";
        return;
    }

    // Print display properties
    for (uint32_t i = 0; i < displayCount; i++) {
        const VkDisplayPropertiesKHR& props = displayProperties[i];
        std::cout << "Display #" << i << ":\n";
        std::cout << "  Display Name: " << (props.displayName ? props.displayName : "Unknown") << "\n";
        std::cout << "  Physical Dimensions: " << props.physicalDimensions.width << "mm x "
                << props.physicalDimensions.height << "mm\n";
        std::cout << "  Physical Resolution: " << props.physicalResolution.width << " x "
                << props.physicalResolution.height << "\n";
        std::cout << "  Supported Transforms: " << props.supportedTransforms << "\n";
        std::cout << "  Plane Reorder Possible: " << (props.planeReorderPossible ? "Yes" : "No") << "\n";
        std::cout << "  Persistent Content: " << (props.persistentContent ? "Yes" : "No") << "\n";
        std::cout << "---------------------------------------------------\n";
    }
}
