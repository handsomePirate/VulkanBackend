#pragma once
#include <Logger/Logger.hpp>

#define DEF_HANDLE(o) typedef struct o ## _T* o;

DEF_HANDLE(VkInstance);
DEF_HANDLE(VkDebugUtilsMessengerEXT);

namespace VulkanBackend
{
	::Core::Logger& GetLogger();

	struct Initialized
	{
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
	};

	Initialized Initialize(const char* configFilePath);

	void Destroy(Initialized& initialized);
}
