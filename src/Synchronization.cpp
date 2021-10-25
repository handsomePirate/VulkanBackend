#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkSemaphore VulkanBackend::CreateSemaphore(const BackendData& backendData)
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkSemaphore semaphore;
	VulkanCheck(vkCreateSemaphore(backendData.logicalDevice, &semaphoreCreateInfo, nullptr, &semaphore));
	
	return semaphore;
}

void VulkanBackend::DestroySemaphore(const BackendData& backendData, VkSemaphore& semaphore)
{
	vkDestroySemaphore(backendData.logicalDevice, semaphore, nullptr);
	semaphore = VK_NULL_HANDLE;
}

VkFence VulkanBackend::CreateFence(const BackendData& backendData, VkFenceCreateFlags flags)
{
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = flags;

	VkFence fence;
	VulkanCheck(vkCreateFence(backendData.logicalDevice, &fenceCreateInfo, nullptr, &fence));
	
	return fence;
}

void VulkanBackend::DestroyFence(const BackendData& backendData, VkFence& fence)
{
	vkDestroyFence(backendData.logicalDevice, fence, nullptr);
	fence = VK_NULL_HANDLE;
}
