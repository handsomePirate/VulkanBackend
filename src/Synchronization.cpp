#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkSemaphore VulkanBackend::CreateSemaphore(VkDevice device)
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkSemaphore semaphore;
	VulkanCheck(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
	
	return semaphore;
}

void VulkanBackend::DestroySemaphore(VkDevice device, VkSemaphore& semaphore)
{
	vkDestroySemaphore(device, semaphore, nullptr);
	semaphore = VK_NULL_HANDLE;
}

VkFence VulkanBackend::CreateFence(VkDevice device, VkFenceCreateFlags flags)
{
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = flags;

	VkFence fence;
	VulkanCheck(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
	
	return fence;
}

void VulkanBackend::DestroyFence(VkDevice device, VkFence& fence)
{
	vkDestroyFence(device, fence, nullptr);
	fence = VK_NULL_HANDLE;
}
