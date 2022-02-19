#pragma once
#include <vulkan/vulkan.h>

namespace VulkanBackend
{
	void VulkanCheckResult(VkResult res, const char* expression, int line, const char* file);
}

#define VulkanCheck(e) (::VulkanBackend::VulkanCheckResult(e, #e, __LINE__, __FILE__))
