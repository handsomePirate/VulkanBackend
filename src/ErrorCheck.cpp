#include "VulkanBackend/ErrorCheck.hpp"
#include <SoftwareCore/DefaultLogger.hpp>
#include <MagicEnum/MagicEnum.hpp>

void VulkanBackend::VulkanCheckResult(VkResult res, const char* expression, int line, const char* file)
{
	if (res != VK_SUCCESS)
	{
		auto enumValue = magic_enum::enum_name(res);
		CoreLogError(DefaultLogger, "Vulkan: %s(%i)\n%s == %s", file, line, expression, enumValue.data());
	}
}
