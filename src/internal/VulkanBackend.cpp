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

void DestroyInstance(VulkanBackend::Initialized& initialized);

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
	VkPhysicalDeviceProperties pickedDeviceProperties{};
	VkPhysicalDeviceFeatures pickedDeviceFeatures{};

	if (configData["Device"])
	{
		uint32_t deviceCount;
		VulkanCheck(vkEnumeratePhysicalDevices(initialized.instance, &deviceCount, nullptr));

		std::vector<VkPhysicalDevice> devices(deviceCount);
		VulkanCheck(vkEnumeratePhysicalDevices(initialized.instance, &deviceCount, devices.data()));

		std::vector<std::string> requiredFeatures;
		if (configData["Device"]["features"])
		{
			std::vector<std::string> requiredFeaturesMessy;
			for (int f = 0; f < configData["Device"]["features"].size(); ++f)
			{
				requiredFeaturesMessy.push_back(configData["Device"]["features"][f].as<std::string>());
			}
			requiredFeatures.resize(requiredFeaturesMessy.size());

			for (int f = 0; f < requiredFeatures.size(); ++f)
			{
				requiredFeatures[f].resize(requiredFeaturesMessy[f].length());
				for (int c = 0; c < requiredFeaturesMessy[f].length(); ++c)
				{
					requiredFeatures[f][c] = tolower(requiredFeaturesMessy[f][c]);
				}
			}
		}

		std::vector<VkPhysicalDeviceProperties> deviceProperties(deviceCount);
		std::vector<VkPhysicalDeviceFeatures> deviceFeatures(deviceCount);
		std::string preferredName = "";
		bool foundPreferredDevice = true;
		if (configData["Device"]["preferred"])
		{
			preferredName = configData["Device"]["preferred"].as<std::string>();
			foundPreferredDevice = false;
		}
		for (int d = 0; d < devices.size(); ++d)
		{
			vkGetPhysicalDeviceProperties(devices[d], &deviceProperties[d]);
			vkGetPhysicalDeviceFeatures(devices[d], &deviceFeatures[d]);
			if (!foundPreferredDevice && deviceProperties[d].deviceName == preferredName)
			{
				if (deviceProperties[d].apiVersion >= vulkanApplicationInfo.apiVersion)
				{
					if (Configurator::CheckFeaturesPresent(deviceFeatures[d], deviceProperties[d], requiredFeatures))
					{
						pickedDevice = devices[d];
						pickedDeviceProperties = deviceProperties[d];
						pickedDeviceFeatures = deviceFeatures[d];
						foundPreferredDevice = true;
						break;
					}
				}
				else
				{
					int major = VK_API_VERSION_MAJOR(deviceProperties[d].apiVersion);
					int minor = VK_API_VERSION_MINOR(deviceProperties[d].apiVersion);
					int patch = VK_API_VERSION_PATCH(deviceProperties[d].apiVersion);
					
					CoreLogInfo(VulkanLogger, 
						"Configuration: Preferred device does not support the selected API version (maximum supported version: %i.%i.%i).",
						major, minor, patch);

					foundPreferredDevice = true;
				}
			}

			if (deviceProperties[d].apiVersion < vulkanApplicationInfo.apiVersion)
			{
				devices[d] = VK_NULL_HANDLE;
			}
		}

		if (!foundPreferredDevice)
		{
			CoreLogInfo(VulkanLogger, "Configuration: Preferred device matching requirements could not be found.");
		}

		if (configData["Device"]["preferred"] && configData["Device"]["preferred-vendor"] && pickedDevice != VK_NULL_HANDLE)
		{
			CoreLogInfo(VulkanLogger, "Configuration: Found preferred device that meets requirements; skipping preferred vendor.");
		}

		if (pickedDevice == VK_NULL_HANDLE)
		{
			uint32_t preferredVendorId = 0;
			if (configData["Device"]["preferred-vendor"])
			{
				preferredVendorId = Configurator::VendorIdFromString(configData["Device"]["preferred-vendor"].as<std::string>());
			}
			;
			std::vector<VkPhysicalDevice> preferredDevices;
			std::vector<VkPhysicalDeviceProperties> preferredDevicesProperties;
			std::vector<VkPhysicalDeviceFeatures> preferredDevicesFeatures;
			std::vector<VkPhysicalDevice> otherDevices;
			std::vector<VkPhysicalDeviceProperties> otherDevicesProperties;
			std::vector<VkPhysicalDeviceFeatures> otherDevicesFeatures;

			for (int d = 0; d < devices.size(); ++d)
			{
				if (devices[d] != VK_NULL_HANDLE)
				{
					if (preferredVendorId > 0 && deviceProperties[d].vendorID == preferredVendorId)
					{
						preferredDevices.push_back(devices[d]);
						preferredDevicesProperties.push_back(deviceProperties[d]);
						preferredDevicesFeatures.push_back(deviceFeatures[d]);
					}
					else
					{
						otherDevices.push_back(devices[d]);
						otherDevicesProperties.push_back(deviceProperties[d]);
						otherDevicesFeatures.push_back(deviceFeatures[d]);
					}
				}
			}

			for (int d = 0; d < preferredDevices.size(); ++d)
			{
				if (preferredDevicesProperties[d].apiVersion >= vulkanApplicationInfo.apiVersion)
				{
					if (Configurator::CheckFeaturesPresent(preferredDevicesFeatures[d], preferredDevicesProperties[d], requiredFeatures))
					{
						pickedDevice = preferredDevices[d];
						pickedDeviceProperties = preferredDevicesProperties[d];
						pickedDeviceFeatures = preferredDevicesFeatures[d];
						CoreLogInfo(VulkanLogger, "Configuration: Found suitable preferred vendor device.");
						break;
					}
				}
			}

			if (pickedDevice == VK_NULL_HANDLE)
			{
				if (preferredVendorId > 0)
				{
					CoreLogInfo(VulkanLogger, "Configuration: No suitable devices found from preferred vendor.");
				}
				for (int d = 0; d < otherDevices.size(); ++d)
				{
					if (otherDevicesProperties[d].apiVersion >= vulkanApplicationInfo.apiVersion)
					{
						if (Configurator::CheckFeaturesPresent(otherDevicesFeatures[d], otherDevicesProperties[d], requiredFeatures))
						{
							pickedDevice = otherDevices[d];
							pickedDeviceProperties = otherDevicesProperties[d];
							pickedDeviceFeatures = otherDevicesFeatures[d];
							CoreLogInfo(VulkanLogger, "Configuration: A satisfactory device found.");
							break;
						}
					}
				}
			}

			if (pickedDevice == VK_NULL_HANDLE)
			{
				CoreLogError(VulkanLogger, "Configuration: No suitable devices available - initialization failed.");
				DestroyInstance(initialized);
				return initialized;
			}
		}
	}

	// Returning the initialized Vulkan structures.
	return initialized;
}

void VulkanBackend::Destroy(Initialized& initialized)
{
	DestroyInstance(initialized);
}

void DestroyInstance(VulkanBackend::Initialized& initialized)
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
