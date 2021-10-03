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
			return VK_MAKE_API_VERSION(0, major, minor, patch);
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

	uint32_t VendorIdFromString(const std::string& name);

	bool CheckFeaturesPresent(const VkPhysicalDeviceFeatures& deviceFeatures, const VkPhysicalDeviceProperties& deviceProperties,
		const std::vector<std::string>& requiredFeatures);
	VkPhysicalDeviceFeatures FeaturesFromString(const std::vector<std::string>& requiredFeatures);

	bool SwapchainRequired(const std::vector<std::string>& deviceExtensions);

	bool CheckQueueSupport(const std::vector<std::pair<std::string, int>>& queueRequirements,
		const std::vector<VkQueueFamilyProperties>& queueProperties,
		const std::vector<VkBool32>& queueSupportsPresent,
		std::vector<int>& outputIndices);

	void ConsolidateQueues(const std::vector<std::pair<std::string, int>>& queueRequirements,
		const std::vector<int>& outputIndices, std::vector<int>& queueIndices, const std::string& queueName);
}
