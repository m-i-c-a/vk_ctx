#include "detail/check.hpp"

#include <vk_ctx/instance.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace
{
    std::vector<const char*> cString(const std::vector<std::string>& val)
    {
        std::vector<const char*> tmp;
        tmp.reserve(val.size());

        for (const auto& elem : val)
            tmp.push_back(elem.c_str());

        return tmp;
    }

    std::vector<VkExtensionProperties> supportedInstanceExtensions()
    {
        uint32_t count = 0;
        vk_ctx::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

        std::vector<VkExtensionProperties> extensions(count);

        if (count != 0u)
            vk_ctx::vkCheck(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));

        return extensions;
    }

    std::vector<VkLayerProperties> supportedInstanceLayers()
    {
        uint32_t count = 0;
        vk_ctx::vkCheck(vkEnumerateInstanceLayerProperties(&count, nullptr));

        std::vector<VkLayerProperties> layers(count);

        if (count != 0u)
            vk_ctx::vkCheck(vkEnumerateInstanceLayerProperties(&count, layers.data()));

        return layers;
    }

    void validateInstanceExtensions(const std::vector<std::string>& requested)
    {
        const auto supported = supportedInstanceExtensions();

        for (const auto& name : requested)
        {
            const auto it = std::find_if(
                supported.begin(),
                supported.end(),
                [&name](const VkExtensionProperties& property)
                {
                    return std::strcmp(property.extensionName, name.c_str()) == 0;
                });

            if (it == supported.end())
                throw std::runtime_error("Unsupported Vulkan instance extension: " + name);
        }
    }

    void validateInstanceLayers(const std::vector<std::string>& requested)
    {
        const auto supported = supportedInstanceLayers();

        for (const auto& name : requested)
        {
            const auto it = std::find_if(
                supported.begin(),
                supported.end(),
                [&name](const VkLayerProperties& property)
                {
                    return std::strcmp(property.layerName, name.c_str()) == 0;
                });

            if (it == supported.end())
                throw std::runtime_error("Unsupported Vulkan instance layer: " + name);
        }
    }
}

namespace vk_ctx
{

Instance::Instance(const InstanceCreateInfo& info)
{
    constexpr uint32_t kApiVersion = VK_API_VERSION_1_3;

    validateInstanceExtensions(info.extensions);
    validateInstanceLayers(info.layers);

    const auto layers = cString(info.layers);
    const auto extensions = cString(info.extensions);

    const VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = nullptr,
        .applicationVersion = 0u,
        .pEngineName = nullptr,
        .engineVersion = 0u,
        .apiVersion = kApiVersion,
    };

    const VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0x0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    vkCheck(vkCreateInstance(&createInfo, nullptr, &handle_));
}

Instance::~Instance()
{
    reset();
}

Instance::Instance(Instance&& other) noexcept
    : handle_(std::exchange(other.handle_, VK_NULL_HANDLE))
{
} 

Instance& Instance::operator= (Instance&& other) noexcept
{
    if (this == &other)
        return *this;

    reset();

    handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);

    return *this;
}

void Instance::reset() noexcept
{
    if (handle_ != VK_NULL_HANDLE)
    {
        vkDestroyInstance(handle_, nullptr);
        handle_ = VK_NULL_HANDLE;
    }
}

}