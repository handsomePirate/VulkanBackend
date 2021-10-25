#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkDeviceMemory VulkanBackend::AllocateMemory(const BackendData& backendData, VkDeviceSize size, uint32_t memoryTypeIndex,
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties, VkMemoryPropertyFlags memoryProperties)
{
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory memory;
	VulkanCheck(vkAllocateMemory(backendData.logicalDevice, &memoryAllocateInfo, nullptr, &memory));

	return memory;
}

void VulkanBackend::FreeMemory(const BackendData& backendData, VkDeviceMemory& memory)
{
	vkFreeMemory(backendData.logicalDevice, memory, nullptr);
	memory = VK_NULL_HANDLE;
}

VkMemoryRequirements VulkanBackend::GetImageMemoryRequirements(const BackendData& backendData, VkImage image)
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(backendData.logicalDevice, image, &memoryRequirements);
	return memoryRequirements;
}

VkMemoryRequirements VulkanBackend::GetBufferMemoryRequirements(const BackendData& backendData, VkBuffer buffer)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(backendData.logicalDevice, buffer, &memoryRequirements);
	return memoryRequirements;
}

void* VulkanBackend::MapMemory(const BackendData& backendData, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags)
{
	void* output;
	VulkanCheck(vkMapMemory(backendData.logicalDevice, memory, offset, size, flags, &output));
	return output;
}

void VulkanBackend::UnmapMemory(const BackendData& backendData, VkDeviceMemory memory)
{
	vkUnmapMemory(backendData.logicalDevice, memory);
}

void VulkanBackend::FlushMemory(const BackendData& backendData, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size)
{
	VkMappedMemoryRange mappedMemoryRange{};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = offset;
	mappedMemoryRange.size = size;
	
	VulkanCheck(vkFlushMappedMemoryRanges(backendData.logicalDevice, 1, &mappedMemoryRange));
}

void VulkanBackend::InvalidateMemory(const BackendData& backendData, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size)
{
	VkMappedMemoryRange mappedMemoryRange{};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = offset;
	mappedMemoryRange.size = size;
	
	VulkanCheck(vkInvalidateMappedMemoryRanges(backendData.logicalDevice, 1, &mappedMemoryRange));
}

void VulkanBackend::BindImageMemory(const BackendData& backendData, VkImage image, VkDeviceMemory memory, VkDeviceSize offset)
{
	VulkanCheck(vkBindImageMemory(backendData.logicalDevice, image, memory, offset));
}
