#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <vk_ctx/device.hpp>

#include <vulkan/vulkan.h>

namespace vk_ctx::detail
{

struct NamedSurfaceHandle
{
    std::string id;
    VkSurfaceKHR handle = VK_NULL_HANDLE;
};

uint32_t validatePhysicalDeviceIndex(uint32_t requestedIndex, uint32_t availableCount);

void validateRequiredExtensions(
    const std::vector<std::string>& requested,
    const std::vector<std::string>& supported);

void validateRequiredFeatures(
    const DeviceFeatureBlocks& requested,
    const DeviceFeatureBlocks& supported);

uint32_t selectQueueFamilyIndex(
    const std::vector<VkQueueFamilyProperties>& queueFamilies,
    VkQueueFlags requiredFlags,
    const std::vector<VkBool32>* presentSupport);

VkSurfaceKHR findSurfaceById(
    const std::string& surfaceId,
    const std::vector<NamedSurfaceHandle>& surfaces);

}
