#pragma once

#include <source_location>
#include <stdexcept>
#include <string>

#include <vulkan/vulkan.h>

namespace vk_ctx
{

inline void vkCheck(
    VkResult result,
    const std::source_location& loc = std::source_location::current())
{
    if (result == VK_SUCCESS)
    {
        return;
    }

    const std::string message =
        std::string("Vulkan call failed at ") +
        loc.file_name() + ":" + std::to_string(loc.line()) +
        " in " + loc.function_name() +
        " with result " + std::to_string(static_cast<int>(result));

    throw std::runtime_error(message);
}

} // namespace vk_ctx
