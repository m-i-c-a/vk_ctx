#include <vk_ctx/surface.hpp>

#include <stdexcept>
#include <utility>

namespace vk_ctx
{

Surface::Surface(VkInstance instance, const SurfaceCreateInfo& info)
	: id_(info.id)
	, instance_(instance)
{
	if (!info.factory)
		throw std::invalid_argument("Surface factory must be provided");

	handle_ = info.factory(instance_);
}

Surface::~Surface()
{
	reset();
}

Surface::Surface(Surface&& other) noexcept
	: id_(std::exchange(other.id_, {}))
	, instance_(std::exchange(other.instance_, VK_NULL_HANDLE))
	, handle_(std::exchange(other.handle_, VK_NULL_HANDLE))
{
}

Surface& Surface::operator=(Surface&& other) noexcept
{
	if (this == &other)
		return *this;

	reset();

	id_ = std::exchange(other.id_, {});
	instance_ = std::exchange(other.instance_, VK_NULL_HANDLE);
	handle_ = std::exchange(other.handle_, VK_NULL_HANDLE);

	return *this;
}

void Surface::reset() noexcept
{
	if (handle_ != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance_, handle_, nullptr);
		handle_ = VK_NULL_HANDLE;
	}

	instance_ = VK_NULL_HANDLE;
	id_.clear();
}

}