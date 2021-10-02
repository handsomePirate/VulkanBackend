#pragma once
#include <yaml-cpp/yaml.h>
#include <vulkan/vulkan.hpp>

namespace Configurator
{
	struct Version
	{
		int major, minor, patch;
		uint32_t Make() const
		{
			return VK_MAKE_VERSION(major, minor, patch);
		}
	};

	struct Constants
	{
		static constexpr const char* defaultName = "unknown";
		static constexpr Version defaultVulkanVersion{ 1, 2, 0 };
		static constexpr Version defaultVersion{ 0, 0, 0 };
	};

	struct ApplicationInfo
	{
		Version vulkanVersion;
		std::string applicationName;
		Version applicationVersion;
		std::string engineName;
		Version engineVersion;
	};

	Version VersionFromString(const std::string& version);

	ApplicationInfo ConfigureApplication(const YAML::Node& applicationData);
	VkApplicationInfo GetVulkanApplicationInfo(const ApplicationInfo& applicationInfo);
}