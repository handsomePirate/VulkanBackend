#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkCommandPool VulkanBackend::CreateCommandPool(const BackendData& backendData, uint32_t queueIndex, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueIndex;
	commandPoolCreateInfo.flags = flags;

	VkCommandPool commandPool;
	VulkanCheck(vkCreateCommandPool(backendData.logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool));
	
	return commandPool;
}

void VulkanBackend::DestroyCommandPool(const BackendData& backendData, VkCommandPool& commandPool)
{
	vkDestroyCommandPool(backendData.logicalDevice, commandPool, nullptr);
	commandPool = VK_NULL_HANDLE;
}

VkCommandBuffer VulkanBackend::AllocateCommandBuffer(const BackendData& backendData, VkCommandPool commandPool,
	VkCommandBufferLevel commandBufferLevel)
{
	VkCommandBuffer commandBuffer;
	AllocateCommandBuffers(backendData, commandPool, &commandBuffer, 1, commandBufferLevel);
	return commandBuffer;
}

void VulkanBackend::FreeCommandBuffer(const BackendData& backendData,VkCommandPool commandPool, VkCommandBuffer& commandBuffer)
{
	FreeCommandBuffers(backendData, commandPool, &commandBuffer, 1);
}

void VulkanBackend::AllocateCommandBuffers(const BackendData& backendData,VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount,
	VkCommandBufferLevel commandBufferLevel)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = bufferCount;

	VulkanCheck(vkAllocateCommandBuffers(backendData.logicalDevice, &commandBufferAllocateInfo, commandBuffers));
}

void VulkanBackend::FreeCommandBuffers(const BackendData& backendData,VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount)
{
	vkFreeCommandBuffers(backendData.logicalDevice, commandPool, bufferCount, commandBuffers);
	memset(commandBuffers, 0, sizeof(VkCommandBuffer) * bufferCount);
}

void VulkanBackend::ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
	vkResetCommandBuffer(commandBuffer, flags);
}

void VulkanBackend::ResetCommandPool(const BackendData& backendData,VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
	vkResetCommandPool(backendData.logicalDevice, commandPool, flags);
}
