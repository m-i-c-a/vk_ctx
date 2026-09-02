#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "detail/device_validation.hpp"

namespace
{

void expectRuntimeErrorWithKeyword(auto&& fn, const std::string& keyword)
{
    try
    {
        fn();
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& error)
    {
        EXPECT_NE(std::string(error.what()).find(keyword), std::string::npos);
    }
}

}

TEST(DeviceValidationTests, rejectsInvalidPhysicalDeviceIndex)
{
    expectRuntimeErrorWithKeyword(
        []
        {
            (void)vk_ctx::detail::validatePhysicalDeviceIndex(2u, 2u);
        },
        "index");
}

TEST(DeviceValidationTests, rejectsUnsupportedExtension)
{
    const std::vector<std::string> requested { "VK_EXT_missing" };
    const std::vector<std::string> supported { "VK_KHR_swapchain" };

    expectRuntimeErrorWithKeyword(
        [&]
        {
            vk_ctx::detail::validateRequiredExtensions(requested, supported);
        },
        "extension");
}

TEST(DeviceValidationTests, rejectsUnsupportedRequiredFeature)
{
    vk_ctx::DeviceFeatureBlocks requested;
    requested.core = vk_ctx::makeCoreFeatures(
        VkPhysicalDeviceFeatures {
            .samplerAnisotropy = VK_TRUE,
        });

    vk_ctx::DeviceFeatureBlocks supported;
    supported.core = vk_ctx::makeCoreFeatures();

    expectRuntimeErrorWithKeyword(
        [&]
        {
            vk_ctx::detail::validateRequiredFeatures(requested, supported);
        },
        "feature");
}

TEST(DeviceValidationTests, rejectsUnsupportedRequiredVk12Feature)
{
    vk_ctx::DeviceFeatureBlocks requested;
    requested.vk12 = vk_ctx::makeVk12Features(
        VkPhysicalDeviceVulkan12Features {
            .timelineSemaphore = VK_TRUE,
        });

    vk_ctx::DeviceFeatureBlocks supported;
    supported.vk12 = vk_ctx::makeVk12Features();

    expectRuntimeErrorWithKeyword(
        [&]
        {
            vk_ctx::detail::validateRequiredFeatures(requested, supported);
        },
        "vk12");
}

TEST(DeviceValidationTests, rejectsUnsupportedFeatureInChainedBlocks)
{
    vk_ctx::DeviceFeatureBlocks requested;
    requested.core = vk_ctx::makeCoreFeatures(
        VkPhysicalDeviceFeatures {
            .samplerAnisotropy = VK_TRUE,
        });

    requested.vk12 = vk_ctx::makeVk12Features(
        VkPhysicalDeviceVulkan12Features {
            .timelineSemaphore = VK_TRUE,
        });

    requested.vk13 = vk_ctx::makeVk13Features(
        VkPhysicalDeviceVulkan13Features {
            .dynamicRendering = VK_TRUE,
        });

    vk_ctx::DeviceFeatureBlocks supported;
    supported.core = vk_ctx::makeCoreFeatures(
        VkPhysicalDeviceFeatures {
            .samplerAnisotropy = VK_TRUE,
        });

    supported.vk12 = vk_ctx::makeVk12Features(
        VkPhysicalDeviceVulkan12Features {
            .timelineSemaphore = VK_TRUE,
        });

    supported.vk13 = vk_ctx::makeVk13Features();

    expectRuntimeErrorWithKeyword(
        [&]
        {
            vk_ctx::detail::validateRequiredFeatures(requested, supported);
        },
        "vk13");
}

TEST(DeviceValidationTests, rejectsWhenNoQueueFamilyMatchesFlags)
{
    const std::vector<VkQueueFamilyProperties> queueFamilies {
        VkQueueFamilyProperties {
            .queueFlags = VK_QUEUE_TRANSFER_BIT,
            .queueCount = 1u,
            .timestampValidBits = 0u,
            .minImageTransferGranularity = VkExtent3D { 1u, 1u, 1u },
        },
    };

    expectRuntimeErrorWithKeyword(
        [&]
        {
            (void)vk_ctx::detail::selectQueueFamilyIndex(
                queueFamilies,
                VK_QUEUE_GRAPHICS_BIT,
                nullptr);
        },
        "queue");
}

TEST(DeviceValidationTests, rejectsUnknownSurfaceId)
{
    const std::vector<vk_ctx::detail::NamedSurfaceHandle> surfaces {
        vk_ctx::detail::NamedSurfaceHandle {
            .id = "main",
            .handle = VK_NULL_HANDLE,
        },
    };

    expectRuntimeErrorWithKeyword(
        [&]
        {
            (void)vk_ctx::detail::findSurfaceById("missing", surfaces);
        },
        "surface");
}

TEST(DeviceValidationTests, rejectsQueueFamilyWithoutPresentSupport)
{
    const std::vector<VkQueueFamilyProperties> queueFamilies {
        VkQueueFamilyProperties {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT,
            .queueCount = 1u,
            .timestampValidBits = 0u,
            .minImageTransferGranularity = VkExtent3D { 1u, 1u, 1u },
        },
    };

    const std::vector<VkBool32> presentSupport { VK_FALSE };

    expectRuntimeErrorWithKeyword(
        [&]
        {
            (void)vk_ctx::detail::selectQueueFamilyIndex(
                queueFamilies,
                VK_QUEUE_GRAPHICS_BIT,
                &presentSupport);
        },
        "queue");
}

TEST(DeviceValidationTests, acceptsValidSelectionPath)
{
    const std::vector<std::string> requestedExtensions { "VK_KHR_swapchain" };
    const std::vector<std::string> supportedExtensions { "VK_KHR_swapchain", "VK_EXT_memory_budget" };

    vk_ctx::DeviceFeatureBlocks requested;
    requested.core = vk_ctx::makeCoreFeatures(
        VkPhysicalDeviceFeatures {
            .samplerAnisotropy = VK_TRUE,
        });

    requested.vk12 = vk_ctx::makeVk12Features(
        VkPhysicalDeviceVulkan12Features {
            .timelineSemaphore = VK_TRUE,
        });

    vk_ctx::DeviceFeatureBlocks supported;
    supported.core = vk_ctx::makeCoreFeatures(
        VkPhysicalDeviceFeatures {
            .samplerAnisotropy = VK_TRUE,
        });

    supported.vk12 = vk_ctx::makeVk12Features(
        VkPhysicalDeviceVulkan12Features {
            .timelineSemaphore = VK_TRUE,
        });

    const std::vector<VkQueueFamilyProperties> queueFamilies {
        VkQueueFamilyProperties {
            .queueFlags = VK_QUEUE_TRANSFER_BIT,
            .queueCount = 1u,
            .timestampValidBits = 0u,
            .minImageTransferGranularity = VkExtent3D { 1u, 1u, 1u },
        },
        VkQueueFamilyProperties {
            .queueFlags = static_cast<VkQueueFlags>(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT),
            .queueCount = 1u,
            .timestampValidBits = 0u,
            .minImageTransferGranularity = VkExtent3D { 1u, 1u, 1u },
        },
    };

    const std::vector<VkBool32> presentSupport { VK_FALSE, VK_TRUE };

    const std::vector<vk_ctx::detail::NamedSurfaceHandle> surfaces {
        vk_ctx::detail::NamedSurfaceHandle {
            .id = "main",
            .handle = VK_NULL_HANDLE,
        },
    };

    EXPECT_NO_THROW(
        {
            vk_ctx::detail::validateRequiredExtensions(requestedExtensions, supportedExtensions);
            vk_ctx::detail::validateRequiredFeatures(requested, supported);
            EXPECT_EQ(vk_ctx::detail::validatePhysicalDeviceIndex(0u, 1u), 0u);
            EXPECT_EQ(vk_ctx::detail::selectQueueFamilyIndex(
                          queueFamilies,
                          VK_QUEUE_GRAPHICS_BIT,
                          &presentSupport),
                      1u);
            EXPECT_EQ(vk_ctx::detail::findSurfaceById("main", surfaces), VK_NULL_HANDLE);
        });
}
