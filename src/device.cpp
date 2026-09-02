#include "detail/check.hpp"
#include "detail/device_validation.hpp"

#include <vk_ctx/device.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <utility>

namespace
{

struct BuiltFeatureChain
{
    VkPhysicalDeviceFeatures2 core {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features = {},
    };

    std::optional<VkPhysicalDeviceVulkan11Features> vk11;
    std::optional<VkPhysicalDeviceVulkan12Features> vk12;
    std::optional<VkPhysicalDeviceVulkan13Features> vk13;
};

void sanitizeFeatureBlockTypes(BuiltFeatureChain& chain)
{
    chain.core.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    chain.core.pNext = nullptr;

    if (chain.vk11.has_value())
    {
        chain.vk11->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        chain.vk11->pNext = nullptr;
    }

    if (chain.vk12.has_value())
    {
        chain.vk12->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        chain.vk12->pNext = nullptr;
    }

    if (chain.vk13.has_value())
    {
        chain.vk13->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        chain.vk13->pNext = nullptr;
    }
}

void linkFeatureChain(BuiltFeatureChain& chain)
{
    void** tail = &chain.core.pNext;

    if (chain.vk11.has_value())
    {
        *tail = &chain.vk11.value();
        tail = &chain.vk11->pNext;
    }

    if (chain.vk12.has_value())
    {
        *tail = &chain.vk12.value();
        tail = &chain.vk12->pNext;
    }

    if (chain.vk13.has_value())
    {
        *tail = &chain.vk13.value();
        tail = &chain.vk13->pNext;
    }

    *tail = nullptr;
}

BuiltFeatureChain buildFeatureChainFromRequested(const vk_ctx::DeviceFeatureBlocks& requested)
{
    BuiltFeatureChain chain;

    if (requested.core.has_value())
        chain.core.features = requested.core->features;

    if (requested.vk11.has_value())
        chain.vk11 = requested.vk11.value();

    if (requested.vk12.has_value())
        chain.vk12 = requested.vk12.value();

    if (requested.vk13.has_value())
        chain.vk13 = requested.vk13.value();

    sanitizeFeatureBlockTypes(chain);
    linkFeatureChain(chain);

    return chain;
}

vk_ctx::DeviceFeatureBlocks toFeatureBlocks(const BuiltFeatureChain& chain)
{
    vk_ctx::DeviceFeatureBlocks blocks;
    blocks.core = chain.core;

    if (chain.vk11.has_value())
        blocks.vk11 = chain.vk11.value();

    if (chain.vk12.has_value())
        blocks.vk12 = chain.vk12.value();

    if (chain.vk13.has_value())
        blocks.vk13 = chain.vk13.value();

    return blocks;
}

BuiltFeatureChain querySupportedFeatureChain(
    VkPhysicalDevice physicalDevice,
    const vk_ctx::DeviceFeatureBlocks& requested)
{
    BuiltFeatureChain supported;

    if (requested.vk11.has_value())
    {
        supported.vk11 = VkPhysicalDeviceVulkan11Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = nullptr,
        };
    }

    if (requested.vk12.has_value())
    {
        supported.vk12 = VkPhysicalDeviceVulkan12Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = nullptr,
        };
    }

    if (requested.vk13.has_value())
    {
        supported.vk13 = VkPhysicalDeviceVulkan13Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr,
        };
    }

    sanitizeFeatureBlockTypes(supported);
    linkFeatureChain(supported);

    vkGetPhysicalDeviceFeatures2(physicalDevice, &supported.core);

    return supported;
}

std::vector<const char*> cString(const std::vector<std::string>& val)
{
    std::vector<const char*> tmp;
    tmp.reserve(val.size());

    for (const auto& elem : val)
        tmp.push_back(elem.c_str());

    return tmp;
}

std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance)
{
    uint32_t count = 0u;
    vk_ctx::vkCheck(vkEnumeratePhysicalDevices(instance, &count, nullptr));

    std::vector<VkPhysicalDevice> devices(count);
    if (count != 0u)
        vk_ctx::vkCheck(vkEnumeratePhysicalDevices(instance, &count, devices.data()));

    return devices;
}

std::vector<std::string> supportedDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32_t count = 0u;
    vk_ctx::vkCheck(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> extensions(count);
    if (count != 0u)
    {
        vk_ctx::vkCheck(
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data()));
    }

    std::vector<std::string> names;
    names.reserve(extensions.size());
    for (const auto& ext : extensions)
        names.emplace_back(ext.extensionName);

    return names;
}

std::vector<VkQueueFamilyProperties> queueFamilyProperties(VkPhysicalDevice physicalDevice)
{
    uint32_t count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    if (count != 0u)
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, families.data());

    return families;
}

std::vector<VkBool32> presentSupportPerQueueFamily(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t queueFamilyCount)
{
    std::vector<VkBool32> presentSupport(queueFamilyCount, VK_FALSE);

    for (uint32_t i = 0u; i < queueFamilyCount; ++i)
    {
        vk_ctx::vkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport[i]));
    }

    return presentSupport;
}

std::vector<vk_ctx::detail::NamedSurfaceHandle> toNamedSurfaces(const std::vector<vk_ctx::DeviceSurface>& surfaces)
{
    std::vector<vk_ctx::detail::NamedSurfaceHandle> names;
    names.reserve(surfaces.size());

    for (const auto& surface : surfaces)
    {
        names.push_back(vk_ctx::detail::NamedSurfaceHandle {
            .id = surface.id,
            .handle = surface.handle,
        });
    }

    return names;
}

} // namespace

namespace vk_ctx::detail
{

template <typename T>
void validateBoolStruct(
    const T& requested,
    const T& supported,
    const VkBool32* requestedStart,
    const VkBool32* supportedStart,
    const std::string& label)
{
    const auto* requestedBytes = reinterpret_cast<const unsigned char*>(&requested);
    const auto* requestedStartBytes = reinterpret_cast<const unsigned char*>(requestedStart);
    const auto* supportedStartBytes = reinterpret_cast<const unsigned char*>(supportedStart);

    const size_t offset = static_cast<size_t>(requestedStartBytes - requestedBytes);
    const size_t boolCount = (sizeof(T) - offset) / sizeof(VkBool32);

    for (size_t i = 0u; i < boolCount; ++i)
    {
        if ((requestedStart[i] == VK_TRUE) && (supportedStart[i] == VK_FALSE))
        {
            throw std::runtime_error("Required Vulkan feature is not supported in " + label);
        }
    }
}

uint32_t validatePhysicalDeviceIndex(uint32_t requestedIndex, uint32_t availableCount)
{
    if (availableCount == 0u)
        throw std::runtime_error("No Vulkan physical devices are available");

    if (requestedIndex >= availableCount)
    {
        throw std::runtime_error(
            "Requested physical device index is out of range: " + std::to_string(requestedIndex));
    }

    return requestedIndex;
}

void validateRequiredExtensions(
    const std::vector<std::string>& requested,
    const std::vector<std::string>& supported)
{
    for (const auto& name : requested)
    {
        const auto it = std::find(supported.begin(), supported.end(), name);
        if (it == supported.end())
            throw std::runtime_error("Unsupported Vulkan device extension: " + name);
    }
}

void validateRequiredFeatures(
    const DeviceFeatureBlocks& requested,
    const DeviceFeatureBlocks& supported)
{
    if (requested.core.has_value())
    {
        if (!supported.core.has_value())
            throw std::runtime_error("Supported Vulkan core feature block is missing");

        validateBoolStruct(
            requested.core->features,
            supported.core->features,
            reinterpret_cast<const VkBool32*>(&requested.core->features),
            reinterpret_cast<const VkBool32*>(&supported.core->features),
            "core features");
    }

    if (requested.vk11.has_value())
    {
        if (!supported.vk11.has_value())
            throw std::runtime_error("Supported Vulkan vk11 feature block is missing");

        validateBoolStruct(
            requested.vk11.value(),
            supported.vk11.value(),
            &requested.vk11->storageBuffer16BitAccess,
            &supported.vk11->storageBuffer16BitAccess,
            "vk11 features");
    }

    if (requested.vk12.has_value())
    {
        if (!supported.vk12.has_value())
            throw std::runtime_error("Supported Vulkan vk12 feature block is missing");

        validateBoolStruct(
            requested.vk12.value(),
            supported.vk12.value(),
            &requested.vk12->samplerMirrorClampToEdge,
            &supported.vk12->samplerMirrorClampToEdge,
            "vk12 features");
    }

    if (requested.vk13.has_value())
    {
        if (!supported.vk13.has_value())
            throw std::runtime_error("Supported Vulkan vk13 feature block is missing");

        validateBoolStruct(
            requested.vk13.value(),
            supported.vk13.value(),
            &requested.vk13->robustImageAccess,
            &supported.vk13->robustImageAccess,
            "vk13 features");
    }
}

uint32_t selectQueueFamilyIndex(
    const std::vector<VkQueueFamilyProperties>& queueFamilies,
    VkQueueFlags requiredFlags,
    const std::vector<VkBool32>* presentSupport)
{
    if (queueFamilies.empty())
        throw std::runtime_error("Physical device has no queue families");

    for (uint32_t i = 0u; i < queueFamilies.size(); ++i)
    {
        const auto& queueFamily = queueFamilies[i];
        if (queueFamily.queueCount == 0u)
            continue;

        if ((queueFamily.queueFlags & requiredFlags) != requiredFlags)
            continue;

        if (presentSupport != nullptr)
        {
            if (presentSupport->size() != queueFamilies.size())
            {
                throw std::runtime_error(
                    "Queue present support data does not match queue family count");
            }

            if ((*presentSupport)[i] != VK_TRUE)
                continue;
        }

        return i;
    }

    throw std::runtime_error("No queue family satisfies requested queue flags/present requirements");
}

VkSurfaceKHR findSurfaceById(
    const std::string& surfaceId,
    const std::vector<NamedSurfaceHandle>& surfaces)
{
    const auto it = std::find_if(
        surfaces.begin(),
        surfaces.end(),
        [&surfaceId](const NamedSurfaceHandle& surface)
        {
            return surface.id == surfaceId;
        });

    if (it == surfaces.end())
        throw std::runtime_error("Unknown surface id requested by queue: " + surfaceId);

    return it->handle;
}

} // namespace vk_ctx::detail

namespace vk_ctx
{

Device::Device(
    VkInstance instance,
    const std::vector<DeviceSurface>& surfaces,
    const DeviceCreateInfo& info)
{
    const auto physicalDevices = enumeratePhysicalDevices(instance);
    const uint32_t physicalDeviceIndex =
        detail::validatePhysicalDeviceIndex(info.physicalDevice, physicalDevices.size());
    physicalDevice_ = physicalDevices[physicalDeviceIndex];

    const auto supportedExtensions = supportedDeviceExtensions(physicalDevice_);
    detail::validateRequiredExtensions(info.extensions, supportedExtensions);

    const auto requestedFeatureChain = buildFeatureChainFromRequested(info.requiredFeatures);
    const auto supportedFeatureChain = querySupportedFeatureChain(physicalDevice_, info.requiredFeatures);
    detail::validateRequiredFeatures(
        toFeatureBlocks(requestedFeatureChain),
        toFeatureBlocks(supportedFeatureChain));

    const auto queueFamilies = queueFamilyProperties(physicalDevice_);

    std::vector<VkBool32> presentSupport;
    const std::vector<VkBool32>* presentSupportPtr = nullptr;

    if (!info.queue.surface.empty())
    {
        const auto namedSurfaces = toNamedSurfaces(surfaces);
        const VkSurfaceKHR surface = detail::findSurfaceById(info.queue.surface, namedSurfaces);
        presentSupport = presentSupportPerQueueFamily(
            physicalDevice_,
            surface,
            static_cast<uint32_t>(queueFamilies.size()));
        presentSupportPtr = &presentSupport;
    }

    const uint32_t queueFamilyIndex =
        detail::selectQueueFamilyIndex(queueFamilies, info.queue.flags, presentSupportPtr);

    constexpr float kQueuePriority = 1.0f;

    const VkDeviceQueueCreateInfo queueCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1u,
        .pQueuePriorities = &kQueuePriority,
    };

    const auto extensions = cString(info.extensions);

    const VkDeviceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &requestedFeatureChain.core,
        .flags = 0u,
        .queueCreateInfoCount = 1u,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0u,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = nullptr,
    };

    vkCheck(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &handle_));
    vkGetDeviceQueue(handle_, queueFamilyIndex, 0u, &queue_);
}

Device::~Device()
{
    reset();
}

Device::Device(Device&& other) noexcept
    : handle_(std::exchange(other.handle_, VK_NULL_HANDLE))
    , physicalDevice_(std::exchange(other.physicalDevice_, VK_NULL_HANDLE))
    , queue_(std::exchange(other.queue_, VK_NULL_HANDLE))
{
}

Device& Device::operator=(Device&& other) noexcept
{
    if (this == &other)
        return *this;

    reset();

    handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);
    physicalDevice_ = std::exchange(other.physicalDevice_, VK_NULL_HANDLE);
    queue_ = std::exchange(other.queue_, VK_NULL_HANDLE);

    return *this;
}

void Device::reset() noexcept
{
    if (handle_ != VK_NULL_HANDLE)
    {
        vkDestroyDevice(handle_, nullptr);
        handle_ = VK_NULL_HANDLE;
    }

    physicalDevice_ = VK_NULL_HANDLE;
    queue_ = VK_NULL_HANDLE;
}

}
