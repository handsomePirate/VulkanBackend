#include "YAMLConfiguration.hpp"
#include "VulkanBackend/Logger.hpp"
#include <SoftwareCore/Logger.hpp>
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

bool Configurator::CheckQueueSupport(const YAML::Node& queueRequirements, const std::vector<VkQueueFamilyProperties>& queueProperties,
	std::vector<int>& outputIndices, std::map<std::string, int>& queueTypeMapping)
{
	// Find general, transfer and compute async.
	// We don't check for present since we don't have the info now.
	queueTypeMapping["transfer"] = INT32_MAX;
	queueTypeMapping["compute"] = INT32_MAX;
	for (int q = 0; q < queueProperties.size(); ++q)
	{
		if (((queueProperties[q].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)) == 
			(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)))
		{
			queueTypeMapping["general"] = q;
		}
		else if (queueProperties[q].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			queueTypeMapping["compute"] = q;
		}
		else if (queueProperties[q].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			queueTypeMapping["transfer"] = q;
		}
	}

	// Assume outputIndices are correctly resized and initialized to 0.
	const int general = queueTypeMapping["general"];
	const int compute = queueTypeMapping["compute"] == INT32_MAX ? queueTypeMapping["general"] : queueTypeMapping["compute"];
	const int transfer = queueTypeMapping["transfer"] == INT32_MAX ? queueTypeMapping["general"] : queueTypeMapping["transfer"];

	queueTypeMapping["compute"] = queueTypeMapping["compute"] == INT32_MAX ? queueTypeMapping["general"] : queueTypeMapping["compute"];
	queueTypeMapping["transfer"] = queueTypeMapping["transfer"] == INT32_MAX ? queueTypeMapping["general"] : queueTypeMapping["transfer"];

	std::vector<uint32_t> allocatedQueues(queueProperties.size());
	memset(allocatedQueues.data(), 0, sizeof(uint32_t) * allocatedQueues.size());

	auto nodeType = queueRequirements.Type();

	if (queueRequirements["general"])
	{
		if (queueProperties[general].queueCount >= allocatedQueues[general])
		{
			outputIndices[general] += queueRequirements["general"].as<int>();
			allocatedQueues[general] += queueRequirements["general"].as<int>();
		}
		else
		{
			return false;
		}
	}

	if (queueRequirements["transfer"])
	{
		if (queueProperties[transfer].queueCount >= allocatedQueues[transfer])
		{
			outputIndices[transfer] += queueRequirements["transfer"].as<int>();
			allocatedQueues[transfer] += queueRequirements["transfer"].as<int>();
		}
		else
		{
			return false;
		}
	}

	if (queueRequirements["compute"])
	{
		if (queueProperties[compute].queueCount >= allocatedQueues[compute])
		{
			outputIndices[compute] += queueRequirements["compute"].as<int>();
			allocatedQueues[compute] += queueRequirements["compute"].as<int>();
		}
		else
		{
			return false;
		}
	}

	return true;
}
