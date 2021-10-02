//#include<vulkan/vulkan.hpp>
#include "VulkanLogger/Logger.hpp"
#include <Logger/Logger.hpp>

#define VulkanCheck(e)\
if (e != VK_SUCCESS)\
{\
	const char* file = __FILE__;\
	int line = __LINE__;\
	const char* expression = #e;\
	/* TODO: Get enum string. */\
	CoreLogError(VulkanLogger, "Vulkan: %s(%i)\n%s", file, line, expression);\
}