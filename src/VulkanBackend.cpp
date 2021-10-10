#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "YAMLConfiguration.hpp"
#include "ErrorCheck.hpp"
#include "VulkanBackend/Logger.hpp"
#include <SoftwareCore/Logger.hpp>
#include <vulkan/vulkan.hpp>
#include <yaml-cpp/yaml.h>

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
	
	// ================================= Instance =================================

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

	if (!initialized.instance)
	{
		return initialized;
	}

	// ============================== Debug Messenger ==============================

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

	// ================================== Device ==================================

	// Retrieving device extensions.
	std::vector<std::string> deviceExtensions;
	auto deviceExtensionsData = configData["Device"]["extensions"];
	if (deviceExtensionsData)
	{
		for (int e = 0; e < deviceExtensionsData.size(); ++e)
		{
			deviceExtensions.push_back(deviceExtensionsData[e].as<std::string>());
		}
	}

	// Converting device extensions to const char*.
	std::vector<const char*> deviceExtensionsChar(deviceExtensions.size());
	for (int e = 0; e < deviceExtensions.size(); ++e)
	{
		deviceExtensionsChar[e] = deviceExtensions[e].c_str();
	}

	// The chosen device and its features and properties.
	VkPhysicalDeviceProperties pickedDeviceProperties{};
	VkPhysicalDeviceFeatures pickedDeviceFeatures{};

	VkPhysicalDeviceFeatures enabledFeatures;

	std::vector<std::vector<int>> outputIndices;
	std::vector<std::map<std::string, int>> indexMappings;
	int deviceIndex;
	int deviceQueueFamilyCount;

	if (configData["Device"])
	{
		// Querying available devices.
		uint32_t deviceCount;
		VulkanCheck(vkEnumeratePhysicalDevices(initialized.instance, &deviceCount, nullptr));

		std::vector<VkPhysicalDevice> devices(deviceCount);
		VulkanCheck(vkEnumeratePhysicalDevices(initialized.instance, &deviceCount, devices.data()));

		// Assembling required features.
		std::vector<std::string> requiredFeatures;
		if (configData["Device"]["features"])
		{
			// First reading from the config.
			std::vector<std::string> requiredFeaturesMessy;
			for (int f = 0; f < configData["Device"]["features"].size(); ++f)
			{
				requiredFeaturesMessy.push_back(configData["Device"]["features"][f].as<std::string>());
			}
			requiredFeatures.resize(requiredFeaturesMessy.size());

			// Making the feature strings all in lower case for easier matching.
			for (int f = 0; f < requiredFeatures.size(); ++f)
			{
				requiredFeatures[f].resize(requiredFeaturesMessy[f].length());
				for (int c = 0; c < requiredFeaturesMessy[f].length(); ++c)
				{
					requiredFeatures[f][c] = tolower(requiredFeaturesMessy[f][c]);
				}
			}
		}

		enabledFeatures = Configurator::FeaturesFromString(requiredFeatures);


		std::vector<std::vector<VkQueueFamilyProperties>> queueProperties(deviceCount);

		outputIndices.resize(deviceCount);
		indexMappings.resize(deviceCount);

		std::vector<VkPhysicalDeviceProperties> deviceProperties(deviceCount);
		std::vector<VkPhysicalDeviceFeatures> deviceFeatures(deviceCount);
		std::string preferredName = "";
		bool foundPreferredDevice = true;
		if (configData["Device"]["preferred"])
		{
			// If preferred device is specified, we will look for it.
			preferredName = configData["Device"]["preferred"].as<std::string>();
			foundPreferredDevice = false;
		}
		for (int d = 0; d < devices.size(); ++d)
		{
			// Retrieving current device properties and features.
			vkGetPhysicalDeviceProperties(devices[d], &deviceProperties[d]);
			vkGetPhysicalDeviceFeatures(devices[d], &deviceFeatures[d]);

			uint32_t familyCount;
			vkGetPhysicalDeviceQueueFamilyProperties(devices[d], &familyCount, NULL);
			queueProperties[d].resize(familyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(devices[d], &familyCount, queueProperties[d].data());
			outputIndices[d].resize(familyCount);
			memset(outputIndices[d].data(), 0, sizeof(int) * familyCount);

			if (!foundPreferredDevice && deviceProperties[d].deviceName == preferredName)
			{
				if (deviceProperties[d].apiVersion >= vulkanApplicationInfo.apiVersion)
				{
					// If the requested API version is supported by the device, checking required features support.
					if (Configurator::CheckFeaturesPresent(deviceFeatures[d], deviceProperties[d], requiredFeatures) &&
						Configurator::CheckQueueSupport(configData["Device"]["queues"], queueProperties[d], outputIndices[d], indexMappings[d]))
					{
						// In this case, we found the preferred device and it fits the requirements.
						initialized.physicalDevice = devices[d];
						pickedDeviceProperties = deviceProperties[d];
						pickedDeviceFeatures = deviceFeatures[d];
						foundPreferredDevice = true;
						deviceIndex = d;
						deviceQueueFamilyCount = familyCount;
						break;
					}
				}
				else
				{
					// Here, the requested API version was higher than the device supports.
					int major = VK_API_VERSION_MAJOR(deviceProperties[d].apiVersion);
					int minor = VK_API_VERSION_MINOR(deviceProperties[d].apiVersion);
					int patch = VK_API_VERSION_PATCH(deviceProperties[d].apiVersion);
					
					CoreLogInfo(VulkanLogger, 
						"Configuration: Preferred device does not support the selected API version (maximum supported version: %i.%i.%i).",
						major, minor, patch);

					foundPreferredDevice = true;
				}
			}

			// In case the preferred device was not found, we also filter out devices that do not support the requested API.
			if (deviceProperties[d].apiVersion < vulkanApplicationInfo.apiVersion)
			{
				devices[d] = VK_NULL_HANDLE;
			}
		}

		// Reporting results from preferred device search.
		if (!foundPreferredDevice)
		{
			CoreLogInfo(VulkanLogger, "Configuration: Preferred device matching requirements could not be found.");
		}

		if (initialized.physicalDevice == VK_NULL_HANDLE)
		{
			// If we do not have a device selected, we split the devices into two groups (preferred vendor and the rest).
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

			// Checking the preferred vendor devices first.
			for (int d = 0; d < preferredDevices.size(); ++d)
			{
				if (preferredDevicesProperties[d].apiVersion >= vulkanApplicationInfo.apiVersion)
				{
					if (Configurator::CheckFeaturesPresent(preferredDevicesFeatures[d], preferredDevicesProperties[d], requiredFeatures) &&
						Configurator::CheckQueueSupport(configData["Device"]["queues"], queueProperties[d], outputIndices[d], indexMappings[d]))
					{
						initialized.physicalDevice = preferredDevices[d];
						pickedDeviceProperties = preferredDevicesProperties[d];
						pickedDeviceFeatures = preferredDevicesFeatures[d];
						deviceIndex = d;
						deviceQueueFamilyCount = (int)queueProperties[d].size();
						CoreLogInfo(VulkanLogger, "Configuration: Found suitable preferred vendor device.");
						break;
					}
				}
			}

			if (initialized.physicalDevice == VK_NULL_HANDLE)
			{
				// Checking the rest of the devices if no suitable preferred vendor device is present.
				if (preferredVendorId > 0)
				{
					CoreLogInfo(VulkanLogger, "Configuration: No suitable devices found from preferred vendor.");
				}
				for (int d = 0; d < otherDevices.size(); ++d)
				{
					if (otherDevicesProperties[d].apiVersion >= vulkanApplicationInfo.apiVersion)
					{
						if (Configurator::CheckFeaturesPresent(otherDevicesFeatures[d], otherDevicesProperties[d], requiredFeatures) &&
							Configurator::CheckQueueSupport(configData["Device"]["queues"], queueProperties[d], outputIndices[d], indexMappings[d]))
						{
							initialized.physicalDevice = otherDevices[d];
							pickedDeviceProperties = otherDevicesProperties[d];
							pickedDeviceFeatures = otherDevicesFeatures[d];
							deviceIndex = d;
							deviceQueueFamilyCount = (int)queueProperties[d].size();
							CoreLogInfo(VulkanLogger, "Configuration: A satisfactory device found.");
							break;
						}
					}
				}
			}

			if (initialized.physicalDevice == VK_NULL_HANDLE)
			{
				// In case no available device fulfills the requirements, we report a failure and the initialization cannot continue.
				CoreLogError(VulkanLogger, "Configuration: No suitable devices available - initialization failed.");
				DestroyInstance(initialized);
				return initialized;
			}
		}
	}

	// TODO: Figure out priorities.
	std::vector<float> queuePriorities;
	int maxCount = 1;
	int countFamilies = 0;

	for (int oi = 0; oi < outputIndices[deviceIndex].size(); ++oi)
	{
		maxCount = (std::max)(maxCount, outputIndices[deviceIndex][oi]);
		if (outputIndices[deviceIndex][oi] > 0 || configData["Device"]["queues"]["present"])
		{
			++countFamilies;
		}
	}

	for (int q = 0; q < maxCount; ++q)
	{
		queuePriorities.push_back(0.f);
	}

	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(countFamilies);

	int f = 0;
	for (int oi = 0; oi < outputIndices[deviceIndex].size(); ++oi)
	{
		if (outputIndices[deviceIndex][oi] == 0 && configData["Device"]["queues"]["present"])
		{
			outputIndices[deviceIndex][oi] = 1;
		}

		if (outputIndices[deviceIndex][oi] > 0)
		{
			deviceQueueCreateInfos[f] = {};
			deviceQueueCreateInfos[f].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfos[f].pQueuePriorities = queuePriorities.data();
			deviceQueueCreateInfos[f].queueFamilyIndex = f;
			deviceQueueCreateInfos[f].queueCount = outputIndices[deviceIndex][oi]; /* 1 for present queue candidates*/
			++f;
		}
	}
	
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsChar.data();
	deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensionsChar.size();
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();

	VulkanCheck(vkCreateDevice(initialized.physicalDevice, &deviceCreateInfo, nullptr, &initialized.logicalDevice));

	std::vector<int> currentQueues(outputIndices[deviceIndex].size());

	initialized.generalQueues.resize(configData["Device"]["queues"]["general"].as<int>());
	for (int i = 0; i < initialized.generalQueues.size(); ++i)
	{
		vkGetDeviceQueue(initialized.logicalDevice, indexMappings[deviceIndex]["general"],
			currentQueues[indexMappings[deviceIndex]["general"]]++, &initialized.generalQueues[i]);
	}
	initialized.generalFamilyIndex = indexMappings[deviceIndex]["general"];
	
	initialized.computeFamilyIndex = 0;
	if (configData["Device"]["queues"]["compute"])
	{
		initialized.computeQueues.resize(configData["Device"]["queues"]["compute"].as<int>());
		for (int i = 0; i < initialized.generalQueues.size(); ++i)
		{
			vkGetDeviceQueue(initialized.logicalDevice, indexMappings[deviceIndex]["compute"],
				currentQueues[indexMappings[deviceIndex]["compute"]]++, &initialized.computeQueues[i]);
		}
		initialized.computeFamilyIndex = indexMappings[deviceIndex]["compute"];
	}

	initialized.transferFamilyIndex = 0;
	if (configData["Device"]["queues"]["transfer"])
	{
		initialized.transferQueues.resize(configData["Device"]["queues"]["transfer"].as<int>());
		for (int i = 0; i < initialized.transferQueues.size(); ++i)
		{
			vkGetDeviceQueue(initialized.logicalDevice, indexMappings[deviceIndex]["transfer"],
				currentQueues[indexMappings[deviceIndex]["transfer"]]++, &initialized.transferQueues[i]);
		}
		initialized.transferFamilyIndex = indexMappings[deviceIndex]["transfer"];
	}

	if (configData["Device"]["queues"]["present"])
	{
		initialized.presentQueueCandidates.resize(currentQueues.size());
		for (int i = 0; i < initialized.presentQueueCandidates.size(); ++i)
		{
			if (i == indexMappings[deviceIndex]["general"])
			{
				initialized.presentQueueCandidates[i] = initialized.generalQueues[0];
			}
			else if (i == indexMappings[deviceIndex]["transfer"] && initialized.transferQueues.size() > 0)
			{
				initialized.presentQueueCandidates[i] = initialized.transferQueues[0];
			}
			else if (i == indexMappings[deviceIndex]["compute"] && initialized.computeQueues.size() > 0)
			{
				initialized.presentQueueCandidates[i] = initialized.computeQueues[0];
			}
			else
			{
				vkGetDeviceQueue(initialized.logicalDevice, i,
					currentQueues[i]++, &initialized.presentQueueCandidates[i]);
			}
		}
	}
	
	// Returning the initialized Vulkan structures.
	return initialized;
}

void VulkanBackend::Shutdown(Initialized& initialized)
{
	initialized.generalFamilyIndex = 0;
	initialized.computeFamilyIndex = 0;
	initialized.transferFamilyIndex = 0;

	initialized.generalQueues.clear();
	initialized.computeQueues.clear();
	initialized.transferQueues.clear();
	initialized.presentQueueCandidates.clear();


	vkDestroyDevice(initialized.logicalDevice, nullptr);
	initialized.logicalDevice = VK_NULL_HANDLE;
	initialized.physicalDevice = VK_NULL_HANDLE;

	DestroyInstance(initialized);
}

VkFormat VulkanBackend::GetDepthFormat(VkPhysicalDevice device)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use.
	// Start with the highest precision packed format.
	std::vector<VkFormat> depthFormats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(device, format, &formatProperties);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

VkSurfaceFormatKHR VulkanBackend::GetSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	// Get list of supported surface formats
	uint32_t formatCount;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
	assert(formatCount > 0 && result == VK_SUCCESS);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());

	VkSurfaceFormatKHR surfaceFormat;

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		// iterate over the list of available surface format and
		// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
		for (auto&& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				surfaceFormat.format = surfaceFormat.format;
				surfaceFormat.colorSpace = surfaceFormat.colorSpace;
				return surfaceFormat;
			}
		}

		// in case VK_FORMAT_B8G8R8A8_UNORM is not available
		// select the first available color format
		surfaceFormat.format = surfaceFormats[0].format;
		surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	}
	return surfaceFormat;
}

VkPhysicalDeviceMemoryProperties VulkanBackend::GetDeviceMemoryProperties(VkPhysicalDevice device)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
	return memoryProperties;
}

void VulkanBackend::FilterPresentQueues(Initialized& initialized, VkSurfaceKHR surface)
{
	int presentQueues = (int)initialized.presentQueueCandidates.size();
	for (int q = 0; q < initialized.presentQueueCandidates.size(); ++q)
	{
		VkBool32 supports;
		vkGetPhysicalDeviceSurfaceSupportKHR(initialized.physicalDevice, q, surface, &supports);

		if (!supports)
		{
			initialized.presentQueueCandidates[q] = VK_NULL_HANDLE;
			--presentQueues;
		}
	}

	if (presentQueues == 0)
	{
		CoreLogError(VulkanLogger, "Vulkan: No present-capable queues available.");
	}
}

VkQueue VulkanBackend::SelectPresentQueue(const Initialized& initialized)
{
	VkQueue generalQueue = initialized.presentQueueCandidates[initialized.generalFamilyIndex];

	if (generalQueue)
	{
		return generalQueue;
	}

	for (int q = 0; q < initialized.presentQueueCandidates.size(); ++q)
	{
		if (initialized.presentQueueCandidates[q])
		{
			return initialized.presentQueueCandidates[q];
		}
	}

	return VK_NULL_HANDLE;
}

VkQueue VulkanBackend::SelectPresentComputeQueue(const Initialized& initialized)
{
	return initialized.presentQueueCandidates[initialized.computeFamilyIndex];
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

	// Figuring out what severity should the message be.
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
