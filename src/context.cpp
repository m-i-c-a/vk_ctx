#include <vk_ctx/context.hpp>

#include <cassert>

namespace vk_ctx
{

Context::Context(const ContextCreateInfo& info)
    : instance_(info.instance)
{
    for (const auto& surfaceInfo : info.surfaces)
        surfaces_.emplace_back(instance_.get(), surfaceInfo);

    std::vector<DeviceSurface> deviceSurfaces;
    deviceSurfaces.reserve(surfaces_.size());

    for (const auto& surface : surfaces_)
    {
        deviceSurfaces.push_back(DeviceSurface {
            .id = surface.id(),
            .handle = surface.get(),
        });
    }

    for (const auto& deviceInfo : info.devices)
        devices_.emplace_back(instance_.get(), deviceSurfaces, deviceInfo);
}

const Device& Context::device(std::size_t index) const noexcept
{
    assert(index < devices_.size());
    return devices_[index];
}

const Surface& Context::surface(std::size_t index) const noexcept
{
    assert(index < surfaces_.size());
    return surfaces_[index];
}

}
