#include <vulkan/vulkan.h>

#include <vk_ctx/context.hpp>

int main()
{
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
                .requiredFeatures = {},
                .queue = {
                    .flags = VK_QUEUE_COMPUTE_BIT,
                    .surface = {},
                },
            },
        },
    };

    const vk_ctx::Context ctx(info);
    (void)ctx;

    return 0;
}