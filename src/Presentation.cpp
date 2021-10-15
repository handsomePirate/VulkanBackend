#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkSwapchainKHR VulkanBackend::CreateSwapchain(VkDevice device, uint32_t width, uint32_t height, VkSurfaceKHR surface)
{
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	// TODO: Fill in with info from initialized.

	VkSwapchainKHR swapchain;
	vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
	return swapchain;	
}
