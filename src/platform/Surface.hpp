#pragma once
#include "VulkanBackendAPI.hpp"
#include "WindowAPI.hpp"

namespace VulkanSurface
{
	VkSurfaceKHR Create(VkInstance instance, WindowHandle windowHandle);
}
