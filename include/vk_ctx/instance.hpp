#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace vk_ctx
{

struct InstanceCreateInfo
{
    std::vector<std::string> layers;
    std::vector<std::string> extensions;
};

class Instance
{
public:
    explicit Instance(const InstanceCreateInfo& info);
    ~Instance();

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;
    Instance(Instance&& other) noexcept; 
    Instance& operator= (Instance&& other) noexcept;

    [[nodiscard]] VkInstance get() const noexcept { return handle_; }

private: 
    VkInstance handle_ = VK_NULL_HANDLE;

    void reset() noexcept;
};

}