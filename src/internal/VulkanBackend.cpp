#include "../VulkanBackendAPI.hpp"
#include "YAMLConfiguration.hpp"
#include "ErrorCheck.hpp"
#include "VulkanLogger/Logger.hpp"
#include <Logger/Logger.hpp>
#include <vulkan/vulkan.hpp>
#include <yaml-cpp/yaml.h>

::Core::Logger& VulkanBackend::GetLogger()
{
	return VulkanLogger;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);

VulkanBackend::Initialized VulkanBackend::Initialize(const char* configFilePath)
{
	// TODO: Allocator.

	// Loading YAML configuration file.
	YAML::Node configData = YAML::LoadFile(configFilePath);
	
	// Retrieving application info from configuration.
	Configurator::ApplicationInfo applicationInfo;
	if (configData["Application"])
	{
		applicationInfo = Configurator::ConfigureApplication(configData["Application"]);
	}
	else
	{
		// Default if data is missing.
		CoreLogWarn(VulkanLogger, "Configuration: Missing application configuration.");
		applicationInfo.vulkanVersion = Configurator::Constants::defaultVulkanVersion;
		applicationInfo.applicationName = Configurator::Constants::defaultName;
		applicationInfo.applicationVersion = Configurator::Constants::defaultVersion;
		applicationInfo.engineName = Configurator::Constants::defaultName;
		applicationInfo.engineVersion = Configurator::Constants::defaultVersion;
	}

	// Creating Vulkan application info structure.
	auto vulkanApplicationInfo = GetVulkanApplicationInfo(applicationInfo);

	// Loading required extensions and validation layers from configuration.
	std::vector<std::string> extensions;
	std::vector<std::string> layers;
	if (configData["Instance"])
	{
		// Retrieving extension.
		bool hasExtensions = false;
		auto extensionsData = configData["Instance"]["extensions"];
		if (extensionsData)
		{
			for (int e = 0; e < extensionsData.size(); ++e)
			{
				hasExtensions = true;
				extensions.push_back(extensionsData[e].as<std::string>());
			}
		}
		// Retrieving layers.
		bool hasLayers = false;
		auto layersData = configData["Instance"]["validation-layers"];
		if (layersData)
		{
			for (int l = 0; l < layersData.size(); ++l)
			{
				hasLayers = true;
				layers.push_back(layersData[l].as<std::string>());
			}
		}

		// Logging if layers or extensions haven"t been found.
		if (!hasExtensions && !hasLayers)
		{
			CoreLogWarn(VulkanLogger, "Configuration: No instance extensions or layers.");
		}
		else if (!hasExtensions)
		{
			CoreLogInfo(VulkanLogger, "Configuration: No instance extensions.");
		}
		else if (!hasLayers)
		{
			CoreLogInfo(VulkanLogger, "Configuration: No instance layers.");
		}
	}
	else
	{
		CoreLogInfo(VulkanLogger, "Configuration: No instance extensions or layers.");
	}

	// TODO: Check that layers & extensions are present.

	// Converting extensions to const char*.
	std::vector<const char*> extensionsChar(extensions.size());
	for (int e = 0; e < extensions.size(); ++e)
	{
		extensionsChar[e] = extensions[e].c_str();
	}

	// Converting layers to const char*.
	std::vector<const char*> layersChar(layers.size());
	for (int l = 0; l < layers.size(); ++l)
	{
		layersChar[l] = layers[l].c_str();
	}

	// Assembling Vulkan instance create info.
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &vulkanApplicationInfo;
	instanceCreateInfo.ppEnabledLayerNames = layersChar.data();
	instanceCreateInfo.enabledLayerCount = (uint32_t)layersChar.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensionsChar.data();
	instanceCreateInfo.enabledExtensionCount = (uint32_t)extensionsChar.size();

	Initialized initialized{};

	// Creating Vulkan instance.
	VulkanCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &initialized.instance));

	if (layers.size() > 0)
	{
		// Enabling debug messenger if any validation layers are present.
		auto VkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(initialized.instance, "vkCreateDebugUtilsMessengerEXT"));

		if (VkCreateDebugUtilsMessengerEXT)
		{
			// Finalizing debug messenger construction if the creation procedure could be retrieved.
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
			debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCreateInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsMessengerCreateInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			debugUtilsMessengerCreateInfo.pfnUserCallback = ValidationCallback;
			VulkanCheck(VkCreateDebugUtilsMessengerEXT(initialized.instance, &debugUtilsMessengerCreateInfo, nullptr, &initialized.debugMessenger));
		}
		else
		{
			// The debug messenger creation fails at this point mostly when the VK_EXT_debug_utils extension is missing from the instance.
			CoreLogError(VulkanLogger, "Vulkan: Failed to create debug messenger (might be missing the \"VK_EXT_debug_utils\" instance extension).");
		}
	}

	// TODO: Debug object names.

	VkPhysicalDevice pickedDevice = VK_NULL_HANDLE;

	if (configData["Device"])
	{
		uint32_t deviceCount;
		VulkanCheck(vkEnumeratePhysicalDevices(initialized.instance, &deviceCount, nullptr));

		std::vector<VkPhysicalDevice> devices(deviceCount);
		VulkanCheck(vkEnumeratePhysicalDevices(initialized.instance, &deviceCount, devices.data()));

		std::vector<VkPhysicalDeviceProperties> deviceProperties(deviceCount);
		std::string preferredName;
		bool foundPreferredDevice = true;
		if (configData["Device"]["preferred"])
		{
			preferredName = configData["Device"]["preferred"].as<std::string>();
			foundPreferredDevice = false;
		}
		for (int d = 0; d < devices.size(); ++d)
		{
			vkGetPhysicalDeviceProperties(devices[d], &deviceProperties[d]);
			if (!foundPreferredDevice && deviceProperties[d].deviceName == preferredName)
			{
				pickedDevice = devices[d];
				foundPreferredDevice = true;
			}
		}

		if (!foundPreferredDevice)
		{
			CoreLogError(VulkanLogger, "Configuration: Preferred device could not be found.");
		}
	}

	// Returning the initialized Vulkan structures.
	return initialized;
}

void VulkanBackend::Destroy(Initialized& initialized)
{
	if (initialized.debugMessenger != VK_NULL_HANDLE)
	{
		// Destroying the debug messenger if one has been created.
		// Assuming that the destruction procedure can be retrieved since the creation must have been found (otherwise the debug messenger would be null).
		auto VkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(initialized.instance, "vkDestroyDebugUtilsMessengerEXT"));
		VkDestroyDebugUtilsMessengerEXT(initialized.instance, initialized.debugMessenger, nullptr);
		initialized.debugMessenger = VK_NULL_HANDLE;
	}
	// Destroying the instance.
	vkDestroyInstance(initialized.instance, nullptr);
	initialized.instance = VK_NULL_HANDLE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	::Core::LoggerSeverity severity;

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		severity = ::Core::LoggerSeverity::Trace;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		severity = ::Core::LoggerSeverity::Info;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity = ::Core::LoggerSeverity::Warn;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		severity = ::Core::LoggerSeverity::Error;
		break;
	}
	
	VulkanLogger.Log(severity, "%s", pCallbackData->pMessage);

	return VK_TRUE;
}
