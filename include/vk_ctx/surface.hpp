#pragma once

#include <functional>
#include <string>

#include <vulkan/vulkan.h>

namespace vk_ctx
{

using SurfaceFactory = std::function<VkSurfaceKHR(VkInstance)>;

struct SurfaceCreateInfo
{
    std::string id {};
    SurfaceFactory factory {};
};

class Surface
{
public:
    Surface(VkInstance instance, const SurfaceCreateInfo& info);
    ~Surface();

    Surface(const Surface&) = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&& other) noexcept;
    Surface& operator=(Surface&& other) noexcept;

    [[nodiscard]] const std::string& id() const noexcept { return id_; }
    [[nodiscard]] VkSurfaceKHR get() const noexcept { return handle_; }

private:
    std::string id_ {};
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR handle_ = VK_NULL_HANDLE;

    void reset() noexcept;
};

}