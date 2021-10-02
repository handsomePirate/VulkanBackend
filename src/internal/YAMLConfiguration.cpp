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
