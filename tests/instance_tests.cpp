#include <gtest/gtest.h>

#include <stdexcept>
#include <type_traits>
#include <utility>

#include <vk_ctx/instance.hpp>

static_assert(!std::is_copy_constructible_v<vk_ctx::Instance>);
static_assert(!std::is_copy_assignable_v<vk_ctx::Instance>);
static_assert(std::is_move_constructible_v<vk_ctx::Instance>);
static_assert(std::is_move_assignable_v<vk_ctx::Instance>);

TEST(InstanceTests, acceptsEmptyConfiguration)
{
    vk_ctx::InstanceCreateInfo info {
        .layers = {},
        .extensions = {},
    };

    EXPECT_NO_THROW({
        const vk_ctx::Instance instance(info);
        EXPECT_NE(instance.get(), VK_NULL_HANDLE);
    });
}

TEST(InstanceTests, rejectsUnsupportedExtension)
{
    vk_ctx::InstanceCreateInfo info {
        .layers = {},
        .extensions = { "VK_KHR_not_a_real_extension" },
    };

    EXPECT_THROW(
        {
            try
            {
                const vk_ctx::Instance instance(info);
                (void)instance;
            }
            catch (const std::runtime_error&)
            {
                throw;
            }
        },
        std::runtime_error);
}

TEST(InstanceTests, rejectsUnsupportedLayer)
{
    vk_ctx::InstanceCreateInfo info {
        .layers = { "VK_LAYER_not_a_real_layer" },
        .extensions = {},
    };

    EXPECT_THROW(
        {
            try
            {
                const vk_ctx::Instance instance(info);
                (void)instance;
            }
            catch (const std::runtime_error&)
            {
                throw;
            }
        },
        std::runtime_error);
}

TEST(InstanceTests, moveConstructsAndTransfersOwnership)
{
    vk_ctx::InstanceCreateInfo info {
        .layers = {},
        .extensions = {},
    };

    vk_ctx::Instance first(info);
    EXPECT_NE(first.get(), VK_NULL_HANDLE);

    vk_ctx::Instance second(std::move(first));

    EXPECT_EQ(first.get(), VK_NULL_HANDLE);
    EXPECT_NE(second.get(), VK_NULL_HANDLE);
}
