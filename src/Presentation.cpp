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

void VulkanBackend::DestroySwapchain(VkDevice device, VkSwapchainKHR& swapchain)
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	swapchain = VK_NULL_HANDLE;
}

void VulkanBackend::GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& images)
{
	uint32_t imageCount;
	VulkanCheck(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

	// Get the swap chain images
	images.resize(imageCount);
	VulkanCheck(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data()));
}

VkFramebuffer VulkanBackend::CreateFramebuffer(VkDevice device, uint32_t width, uint32_t height, VkRenderPass renderPass, const std::vector<VkImageView>& attachments)
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
	VulkanCheck(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer));
	return framebuffer;
}

void VulkanBackend::DestroyFramebuffer(VkDevice device, VkFramebuffer& framebuffer)
{
	vkDestroyFramebuffer(device, framebuffer, nullptr);
	framebuffer = VK_NULL_HANDLE;
}
