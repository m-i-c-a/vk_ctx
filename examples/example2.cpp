#include <vulkan/vulkan.h>

#include <vk_ctx/context.hpp>

int main()
{
    vk_ctx::DeviceFeatureBlocks features;

    features.core = vk_ctx::makeCoreFeatures(
        VkPhysicalDeviceFeatures {
            .samplerAnisotropy = VK_TRUE,
        });

    features.vk12 = vk_ctx::makeVk12Features(
        VkPhysicalDeviceVulkan12Features {
            .timelineSemaphore = VK_TRUE,
        });

    features.vk13 = vk_ctx::makeVk13Features(
        VkPhysicalDeviceVulkan13Features {
            .dynamicRendering = VK_TRUE,
        });

    const vk_ctx::ContextCreateInfo info {
        .instance = {
            .layers = {},
            .extensions = {},
        },
        .surfaces = {},
        .devices = {
            {
                .physicalDevice = 0u,
                .extensions = {},
                .requiredFeatures = features,
                .queue = {
                    .flags = VK_QUEUE_GRAPHICS_BIT,
                    .surface = {},
                },
            },
        },
    };

    const vk_ctx::Context ctx(info);
    (void)ctx;

    return 0;
}