#include "VulkanBackend/Logger.hpp"
#include <SoftwareCore/Logger.hpp>
#include <MagicEnum/MagicEnum.hpp>

#define VulkanCheck(e)\
{\
VkResult res = e;\
if (res != VK_SUCCESS)\
{\
	const char* file = __FILE__;\
	int line = __LINE__;\
	const char* expression = #e;\
	/* TODO: Get enum string. */\
	CoreLogError(VulkanLogger, "Vulkan: %s(%i)\n%s == %s", file, line, expression, magic_enum::enum_name(res).data());\
}\
}