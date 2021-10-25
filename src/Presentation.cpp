#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkSwapchainKHR VulkanBackend::CreateSwapchain(const BackendData& backendData, const SurfaceData& surfaceData,
	VkImageUsageFlags imageUsage, VkSharingMode sharingMode)
{
	VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
	return RecreateSwapchain(backendData, surfaceData, oldSwapchain, imageUsage, sharingMode);
}

VkSwapchainKHR VulkanBackend::RecreateSwapchain(const BackendData& backendData, const SurfaceData& surfaceData,
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

	// TODO: Support transfer source and destination if necessary and possible.

	VkSwapchainKHR swapchain;
	VulkanCheck(vkCreateSwapchainKHR(backendData.logicalDevice, &swapchainCreateInfo, nullptr, &swapchain));
	return swapchain;
}

void VulkanBackend::DestroySwapchain(const BackendData& backendData, VkSwapchainKHR& swapchain)
{
	vkDestroySwapchainKHR(backendData.logicalDevice, swapchain, nullptr);
	swapchain = VK_NULL_HANDLE;
}

void VulkanBackend::GetSwapchainImages(const BackendData& backendData, VkSwapchainKHR swapchain, std::vector<VkImage>& images)
{
	uint32_t imageCount;
	VulkanCheck(vkGetSwapchainImagesKHR(backendData.logicalDevice, swapchain, &imageCount, nullptr));

	// Get the swap chain images
	images.resize(imageCount);
	VulkanCheck(vkGetSwapchainImagesKHR(backendData.logicalDevice, swapchain, &imageCount, images.data()));
}

VkFramebuffer VulkanBackend::CreateFramebuffer(const BackendData& backendData, uint32_t width, uint32_t height, VkRenderPass renderPass, const std::vector<VkImageView>& attachments)
{
	VkFramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = width;
	framebufferCreateInfo.height = height;
	framebufferCreateInfo.layers = 1;

	VkFramebuffer framebuffer;
	VulkanCheck(vkCreateFramebuffer(backendData.logicalDevice, &framebufferCreateInfo, nullptr, &framebuffer));
	return framebuffer;
}

void VulkanBackend::DestroyFramebuffer(const BackendData& backendData, VkFramebuffer& framebuffer)
{
	vkDestroyFramebuffer(backendData.logicalDevice, framebuffer, nullptr);
	framebuffer = VK_NULL_HANDLE;
}
