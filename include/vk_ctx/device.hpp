#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>


namespace vk_ctx
{

struct QueueRequest
{
    VkQueueFlags flags {};
    std::string surface;
};

struct DeviceSurface
{
    std::string id;
    VkSurfaceKHR handle = VK_NULL_HANDLE;
};

struct DeviceFeatureBlocks
{
    std::optional<VkPhysicalDeviceFeatures2> core;
    std::optional<VkPhysicalDeviceVulkan11Features> vk11;
    std::optional<VkPhysicalDeviceVulkan12Features> vk12;
    std::optional<VkPhysicalDeviceVulkan13Features> vk13;
};

[[nodiscard]] inline VkPhysicalDeviceFeatures2 makeCoreFeatures(
    const VkPhysicalDeviceFeatures& features = {}) noexcept
{
    return VkPhysicalDeviceFeatures2 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features = features,
    };
}

[[nodiscard]] inline VkPhysicalDeviceVulkan11Features makeVk11Features(
    VkPhysicalDeviceVulkan11Features features = {}) noexcept
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features.pNext = nullptr;
    return features;
}

[[nodiscard]] inline VkPhysicalDeviceVulkan12Features makeVk12Features(
    VkPhysicalDeviceVulkan12Features features = {}) noexcept
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features.pNext = nullptr;
    return features;
}

[[nodiscard]] inline VkPhysicalDeviceVulkan13Features makeVk13Features(
    VkPhysicalDeviceVulkan13Features features = {}) noexcept
{
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features.pNext = nullptr;
    return features;
}

struct DeviceCreateInfo
{
    uint32_t physicalDevice = 0u;
    std::vector<std::string> extensions;
    DeviceFeatureBlocks requiredFeatures {};
    QueueRequest queue {};
};

class Device
{
public:
    Device(
        VkInstance instance,
        const std::vector<DeviceSurface>& surfaces,
        const DeviceCreateInfo& info);
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&& other) noexcept;
    Device& operator=(Device&& other) noexcept;

    [[nodiscard]] VkDevice get() const noexcept { return handle_; }
    [[nodiscard]] VkPhysicalDevice physicalDevice() const noexcept { return physicalDevice_; }
    [[nodiscard]] VkQueue queue() const noexcept { return queue_; }

private:
    VkDevice handle_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;

    void reset() noexcept;
};

}