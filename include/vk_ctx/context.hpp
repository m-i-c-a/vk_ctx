#pragma once

#include <vector>

#include "device.hpp"
#include "instance.hpp"
#include "surface.hpp"

namespace vk_ctx
{

struct ContextCreateInfo
{
    InstanceCreateInfo instance {};
    std::vector<SurfaceCreateInfo> surfaces;
    std::vector<DeviceCreateInfo> devices;
};

class Context
{
public:
    explicit Context(const ContextCreateInfo& info);

    [[nodiscard]] const Device& device(std::size_t index) const noexcept;

    [[nodiscard]] const Surface& surface(std::size_t index) const noexcept;

private:
    Instance instance_;
    std::vector<Surface> surfaces_;
    std::vector<Device> devices_;
};



} // namespace vkctx