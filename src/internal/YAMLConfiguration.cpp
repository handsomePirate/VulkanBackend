#include "YAMLConfiguration.hpp"
#include "VulkanLogger/Logger.hpp"
#include <Logger/Logger.hpp>
#include <regex>

Configurator::Version Configurator::VersionFromString(const std::string& version)
{
	std::regex versionAllRegex("[0-9]+\\.[0-9]+\\.[0-9]+");
	std::regex versionMajorMinorRegex("[0-9]+\\.[0-9]+");
	if (!std::regex_match(version, versionAllRegex) && !std::regex_match(version, versionMajorMinorRegex))
	{
		CoreLogError(VulkanLogger, "Configuration: Wrong version format.");
		return { 0, 0, 0 };
	}

	std::regex versionNumberRegex("[0-9]+");
	std::smatch stringMatches;
	std::vector<int> versionParts;
	int m = 0;
	std::string versionMutable = version;
	while (std::regex_search(versionMutable, stringMatches, versionNumberRegex))
	{
		versionParts.push_back(stoi(stringMatches.str(0)));
		versionMutable = stringMatches.suffix();
	}

	return Version{ versionParts[0], versionParts[1], versionParts.size() > 2 ? versionParts[2] : 0};
}

Configurator::ApplicationInfo Configurator::ConfigureApplication(const YAML::Node& applicationData)
{
	ApplicationInfo applicationInfo{};
	
	if (!applicationData["vulkan-version"])
	{
		CoreLogWarn(VulkanLogger, "Configuration: Missing Vulkan API version (default = 1.2.0).");
	}

	if (!applicationData["name"])
	{
		CoreLogWarn(VulkanLogger, "Configuration: Missing application name (default = \"unknown\").");
	}

	if (!applicationData["version"])
	{
		CoreLogWarn(VulkanLogger, "Configuration: Missing application version (default = 0.0.0).");
	}

	if (!applicationData["engine-name"])
	{
		CoreLogWarn(VulkanLogger, "Configuration: Missing engine name (default = \"unknown\").");
	}

	if (!applicationData["engine-version"])
	{
		CoreLogWarn(VulkanLogger, "Configuration: Missing engine version (default = 0.0.0).");
	}

	applicationInfo.vulkanVersion = applicationData["vulkan-version"] ?
		VersionFromString(applicationData["vulkan-version"].as<std::string>()) :
		Constants::defaultVulkanVersion;

	applicationInfo.applicationName = applicationData["name"] ?
		applicationData["name"].as<std::string>() :
		Constants::defaultName;
	
	applicationInfo.applicationVersion = applicationData["version"] ?
		VersionFromString(applicationData["version"].as<std::string>()) :
		Constants::defaultVersion;

	applicationInfo.engineName = applicationData["engine-name"] ?
		applicationData["engine-name"].as<std::string>() :
		Constants::defaultName;
	
	applicationInfo.engineVersion = applicationData["engine-version"] ?
		VersionFromString(applicationData["engine-version"].as<std::string>()) :
		Constants::defaultVersion;

	return applicationInfo;
}

VkApplicationInfo Configurator::GetVulkanApplicationInfo(const ApplicationInfo& applicationInfo)
{
	VkApplicationInfo vulkanApplicationInfo{};
	vulkanApplicationInfo.apiVersion = applicationInfo.vulkanVersion.Make();
	vulkanApplicationInfo.pApplicationName = applicationInfo.applicationName.c_str();
	vulkanApplicationInfo.applicationVersion = applicationInfo.applicationVersion.Make();
	vulkanApplicationInfo.pEngineName = applicationInfo.engineName.c_str();
	vulkanApplicationInfo.engineVersion = applicationInfo.engineVersion.Make();

	return vulkanApplicationInfo;
}

uint32_t Configurator::VendorIdFromString(const std::string& name)
{
	std::string nameLower;
	nameLower.resize(name.length());

	for (int c = 0; c < name.length(); ++c)
	{
		nameLower[c] = tolower(name[c]);
	}

	if (nameLower == "amd" || nameLower == "advanced micro devices")
	{
		return 0x1002;
	}

	if (nameLower == "imgtec" || nameLower == "imagination technologies")
	{
		return 0x1010;
	}

	if (nameLower == "nvda" || nameLower == "nvidia")
	{
		return 0x10DE;
	}

	if (nameLower == "arm" || nameLower == "arm holdings")
	{
		return 0x13B5;
	}

	if (nameLower == "qualcomm")
	{
		return 0x5143;
	}

	if (nameLower == "intel" )
	{
		return 0x8086;
	}

	return 0;
}

bool Configurator::CheckFeaturesPresent(const VkPhysicalDeviceFeatures& deviceFeatures, const VkPhysicalDeviceProperties& deviceProperties,
	const std::vector<std::string>& requiredFeatures)
{
	// TODO: discrete from bool and the rest from actual VkPhysicalDeviceFeatures.
	if ((std::find(requiredFeatures.begin(), requiredFeatures.end(), "dedicated") != requiredFeatures.end() ||
		std::find(requiredFeatures.begin(), requiredFeatures.end(), "discrete") != requiredFeatures.end()) &&
		deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return false;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "geometry shader") != requiredFeatures.end() &&
		deviceFeatures.geometryShader != VK_TRUE)
	{
		return false;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "sparse binding") != requiredFeatures.end() &&
		deviceFeatures.sparseBinding != VK_TRUE)
	{
		return false;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "tessellation shader") != requiredFeatures.end() &&
		deviceFeatures.tessellationShader != VK_TRUE)
	{
		return false;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "wide lines") != requiredFeatures.end() &&
		deviceFeatures.wideLines != VK_TRUE)
	{
		return false;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "int64") != requiredFeatures.end() &&
		deviceFeatures.shaderInt64 != VK_TRUE)
	{
		return false;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "float64") != requiredFeatures.end() &&
		deviceFeatures.shaderFloat64 != VK_TRUE)
	{
		return false;
	}
	
	return true;
}

VkPhysicalDeviceFeatures Configurator::FeaturesFromString(const std::vector<std::string>& requiredFeatures)
{
	VkPhysicalDeviceFeatures result{};

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "geometry shader") != requiredFeatures.end())
	{
		result.geometryShader = VK_TRUE;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "sparse binding") != requiredFeatures.end())
	{
		result.sparseBinding = VK_TRUE;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "tessellation shader") != requiredFeatures.end())
	{
		result.tessellationShader = VK_TRUE;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "wide lines") != requiredFeatures.end())
	{
		result.wideLines = VK_TRUE;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "int64") != requiredFeatures.end())
	{
		result.shaderInt64 = VK_TRUE;
	}

	if (std::find(requiredFeatures.begin(), requiredFeatures.end(), "float64") != requiredFeatures.end())
	{
		result.shaderFloat64 = VK_TRUE;
	}

	return result;
}

bool Configurator::SwapchainRequired(const std::vector<std::string>& deviceExtensions)
{
	std::regex swapchainRegex(".+(_swapchain).*");
	for (int e = 0; e < deviceExtensions.size(); ++e)
	{
		if (std::regex_match(deviceExtensions[e], swapchainRegex))
		{
			return true;
		}
	}
	return false;
}

bool Configurator::CheckQueueSupport(const std::vector<std::pair<std::string, int>>& queueRequirements,
	const std::vector<VkQueueFamilyProperties>& queueProperties,
	const std::vector<VkBool32>& queueSupportsPresent,
	std::vector<int>& outputIndices)
{
	// TODO: Queues from the same family still count as distinct. - No need to get distinct families as well.
	
	std::vector<int> queuesRemaining;
	std::vector<bool> taken;
	for (int q = 0; q < queueProperties.size(); ++q)
	{
		taken.push_back(false);
		queuesRemaining.push_back(queueProperties[q].queueCount);
	}

	std::vector<int> graphicsIndices;
	std::vector<int> computeIndices;
	std::vector<int> transferIndices;
	std::vector<int> presentIndices;
	for (int q = 0; q < queueProperties.size(); ++q)
	{
		if (queueProperties[q].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsIndices.push_back(q);
		}
		if (queueProperties[q].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			computeIndices.push_back(q);
		}
		if (queueProperties[q].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			transferIndices.push_back(q);
		}
		if (queueSupportsPresent[q])
		{
			presentIndices.push_back(q);
		}
	}

	auto queueFunc = [&](const std::vector<int>& indices, int qr, bool& found)
		-> int
	{
		for (int i = 0; i < indices.size(); ++i)
		{
			if (queuesRemaining[indices[i]] >= queueRequirements[qr].second &&
				!taken[indices[i]])
			{
				queuesRemaining[indices[i]] -= queueRequirements[qr].second;
				taken[indices[i]] = true;
				found = true;
				return indices[i];
			}
		}
		for (int i = 0; i < indices.size(); ++i)
		{
			if (queuesRemaining[indices[i]] >= queueRequirements[qr].second)
			{
				queuesRemaining[indices[i]] -= queueRequirements[qr].second;
				found = true;
				return indices[i];
			}
		}
	};

	outputIndices.resize(queueRequirements.size());

	for (int qr = 0; qr < queueRequirements.size(); ++qr)
	{
		bool found = false;
		if (queueRequirements[qr].first == "graphics")
		{
			outputIndices[qr] = queueFunc(graphicsIndices, qr, found);
		}
		else if (queueRequirements[qr].first == "compute")
		{
			outputIndices[qr] = queueFunc(computeIndices, qr, found);
		}
		else if (queueRequirements[qr].first == "transfer")
		{
			outputIndices[qr] = queueFunc(transferIndices, qr, found);
		}
		else if (queueRequirements[qr].first == "present")
		{
			outputIndices[qr] = queueFunc(presentIndices, qr, found);
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}

void Configurator::ConsolidateQueues(const std::vector<std::pair<std::string, int>>& queueRequirements, const std::vector<int>& outputIndices,
	std::vector<int>& queueIndices, const std::string& queueName)
{
	for (int qr = 0; qr < queueRequirements.size(); ++qr)
	{
		if (queueRequirements[qr].first == queueName)
		{
			queueIndices.push_back(outputIndices[qr]);
		}
	}
}
