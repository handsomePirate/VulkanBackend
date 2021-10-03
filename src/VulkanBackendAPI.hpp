#pragma once
#include <Logger/Logger.hpp>

#define DEF_HANDLE(o) typedef struct o ## _T* o;

DEF_HANDLE(VkInstance);
DEF_HANDLE(VkDebugUtilsMessengerEXT);
DEF_HANDLE(VkPhysicalDevice);
DEF_HANDLE(VkDevice);

namespace VulkanBackend
{
	::Core::Logger& GetLogger();

	struct Initialized
	{
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	};

	Initialized Initialize(const char* configFilePath);

	void Destroy(Initialized& initialized);
}
