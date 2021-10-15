#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkSwapchainKHR VulkanBackend::CreateSwapchain(const BackendData& backendData, uint32_t width, uint32_t height, const SurfaceData& surfaceData,
	VkImageUsageFlags imageUsage, VkSharingMode sharingMode)
{
	VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
	return RecreateSwapchain(backendData, width, height, surfaceData, oldSwapchain, imageUsage, sharingMode);
}

VkSwapchainKHR VulkanBackend::RecreateSwapchain(const BackendData& backendData, uint32_t width, uint32_t height, const SurfaceData& surfaceData,
	VkSwapchainKHR& oldSwapchain, VkImageUsageFlags imageUsage, VkSharingMode sharingMode)
{
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageExtent = surfaceData.surfaceExtent;
	swapchainCreateInfo.surface = surfaceData.surface;
	swapchainCreateInfo.imageFormat = surfaceData.surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceData.surfaceFormat.colorSpace;
	swapchainCreateInfo.compositeAlpha = surfaceData.compositeAlpha;
	swapchainCreateInfo.imageUsage = imageUsage;
	swapchainCreateInfo.imageSharingMode = sharingMode;
	swapchainCreateInfo.preTransform = surfaceData.surfaceTransform;
	swapchainCreateInfo.presentMode = surfaceData.presentMode;
	swapchainCreateInfo.oldSwapchain = oldSwapchain;
	oldSwapchain = VK_NULL_HANDLE;
	swapchainCreateInfo.minImageCount = surfaceData.swapchainImageCount;

	VkSwapchainKHR swapchain;
	VulkanCheck(vkCreateSwapchainKHR(backendData.logicalDevice, &swapchainCreateInfo, nullptr, &swapchain));
	return swapchain;
}

void VulkanBackend::DestroySwapchain(VkDevice device, VkSwapchainKHR swapchain)
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}
