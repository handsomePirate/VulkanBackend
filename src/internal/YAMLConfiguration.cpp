#include "YAMLConfiguration.hpp"
#include "VulkanLogger/Logger.hpp"
#include <Logger/Logger.hpp>

Configurator::Version Configurator::VersionFromString(const std::string& version)
{
	// TODO: Fool-proof.
	auto firstDot = version.find('.');
	std::string majorString = version.substr(0, firstDot);
	std::string rest = version.substr(firstDot + 1, version.length() - firstDot - 1);

	auto secondDot = rest.find('.');
	std::string minorString = rest.substr(0, secondDot);
	std::string patchString = rest.substr(secondDot + 1, rest.length() - secondDot - 1);

	return Version{stoi(majorString), stoi(minorString), stoi(patchString)};
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
}

bool Configurator::CheckFeaturesPresent(const VkPhysicalDeviceFeatures& deviceFeatures, const VkPhysicalDeviceProperties& deviceProperties,
	const std::vector<std::string>& requiredFeatures)
{
	if ((std::find(requiredFeatures.begin(), requiredFeatures.end(), "dedicated") != requiredFeatures.end() ||
		std::find(requiredFeatures.begin(), requiredFeatures.end(), "discrete") != requiredFeatures.end()) &&
		deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return false;
	}

	return true;
}
