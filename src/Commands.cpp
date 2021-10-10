#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "ErrorCheck.hpp"

VkCommandPool VulkanBackend::CreateCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueIndex;
	commandPoolCreateInfo.flags = flags;

	VkCommandPool commandPool;
	VulkanCheck(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool));
	
	return commandPool;
}

void VulkanBackend::DestroyCommandPool(VkDevice device, VkCommandPool& commandPool)
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	commandPool = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanBackend::AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool,
	VkCommandBufferLevel commandBufferLevel)
{
	VkCommandBuffer commandBuffer;
	AllocateCommandBuffers(device, commandPool, &commandBuffer, 1, commandBufferLevel);
	return commandBuffer;
}

void VulkanBackend::FreeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer& commandBuffer)
{
	FreeCommandBuffers(device, commandPool, &commandBuffer, 1);
}

void VulkanBackend::AllocateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount,
	VkCommandBufferLevel commandBufferLevel)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = bufferCount;

	VulkanCheck(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers));
}

void VulkanBackend::FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount)
{
	vkFreeCommandBuffers(device, commandPool, bufferCount, commandBuffers);
	memset(commandBuffers, 0, sizeof(VkCommandBuffer) * bufferCount);
}

void VulkanBackend::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
	vkResetCommandBuffer(commandBuffer, flags);
}

void VulkanBackend::ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
	vkResetCommandPool(device, commandPool, flags);
}
