#include <gtest/gtest.h>

#include <stdexcept>
#include <type_traits>
#include <utility>

#include <vk_ctx/instance.hpp>
#include <vk_ctx/surface.hpp>

static_assert(!std::is_default_constructible_v<vk_ctx::Surface>);
static_assert(!std::is_copy_constructible_v<vk_ctx::Surface>);
static_assert(!std::is_copy_assignable_v<vk_ctx::Surface>);
static_assert(std::is_move_constructible_v<vk_ctx::Surface>);
static_assert(std::is_move_assignable_v<vk_ctx::Surface>);

TEST(SurfaceTests, storesIdAndCallsFactoryWithInstanceHandle)
{
    vk_ctx::InstanceCreateInfo instanceInfo {
        .layers = {},
        .extensions = {},
    };

    const vk_ctx::Instance instance(instanceInfo);

    bool called = false;
    VkInstance receivedInstance = VK_NULL_HANDLE;

    vk_ctx::SurfaceCreateInfo surfaceInfo {
        .id = "main",
        .factory =
            [&](VkInstance vkInstance)
            {
                called = true;
                receivedInstance = vkInstance;
                return VK_NULL_HANDLE;
            },
    };

    const vk_ctx::Surface surface(instance.get(), surfaceInfo);

    EXPECT_TRUE(called);
    EXPECT_EQ(receivedInstance, instance.get());
    EXPECT_EQ(surface.id(), "main");
    EXPECT_EQ(surface.get(), VK_NULL_HANDLE);
}

TEST(SurfaceTests, moveConstructsAndTransfersOwnershipAndId)
{
    vk_ctx::InstanceCreateInfo instanceInfo {
        .layers = {},
        .extensions = {},
    };

    const vk_ctx::Instance instance(instanceInfo);

    vk_ctx::SurfaceCreateInfo surfaceInfo {
        .id = "present",
        .factory =
            [](VkInstance)
            {
                return VK_NULL_HANDLE;
            },
    };

    vk_ctx::Surface first(instance.get(), surfaceInfo);
    vk_ctx::Surface second(std::move(first));

    EXPECT_TRUE(first.id().empty());
    EXPECT_EQ(first.get(), VK_NULL_HANDLE);

    EXPECT_EQ(second.id(), "present");
    EXPECT_EQ(second.get(), VK_NULL_HANDLE);
}

TEST(SurfaceTests, moveAssignsAndTransfersOwnershipAndId)
{
    vk_ctx::InstanceCreateInfo instanceInfo {
        .layers = {},
        .extensions = {},
    };

    const vk_ctx::Instance instance(instanceInfo);

    vk_ctx::SurfaceCreateInfo firstInfo {
        .id = "first",
        .factory =
            [](VkInstance)
            {
                return VK_NULL_HANDLE;
            },
    };

    vk_ctx::SurfaceCreateInfo secondInfo {
        .id = "second",
        .factory =
            [](VkInstance)
            {
                return VK_NULL_HANDLE;
            },
    };

    vk_ctx::Surface first(instance.get(), firstInfo);
    vk_ctx::Surface second(instance.get(), secondInfo);

    second = std::move(first);

    EXPECT_TRUE(first.id().empty());
    EXPECT_EQ(first.get(), VK_NULL_HANDLE);

    EXPECT_EQ(second.id(), "first");
    EXPECT_EQ(second.get(), VK_NULL_HANDLE);
}

TEST(SurfaceTests, rejectsMissingFactory)
{
    vk_ctx::InstanceCreateInfo instanceInfo {
        .layers = {},
        .extensions = {},
    };

    const vk_ctx::Instance instance(instanceInfo);

    vk_ctx::SurfaceCreateInfo surfaceInfo {
        .id = "main",
        .factory = {},
    };

    EXPECT_THROW(
        {
            try
            {
                const vk_ctx::Surface surface(instance.get(), surfaceInfo);
                (void)surface;
            }
            catch (const std::invalid_argument&)
            {
                throw;
            }
        },
        std::invalid_argument);
}