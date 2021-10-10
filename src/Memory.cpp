#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "ErrorCheck.hpp"

VkDeviceMemory VulkanBackend::AllocateMemory(VkDevice device, VkDeviceSize size, uint32_t memoryTypeIndex,
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties, VkMemoryPropertyFlags memoryProperties)
{
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory memory;
	VulkanCheck(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory));

	return memory;
}

void VulkanBackend::FreeMemory(VkDevice device, VkDeviceMemory& memory)
{
	vkFreeMemory(device, memory, nullptr);
	memory = VK_NULL_HANDLE;
}

VkMemoryRequirements VulkanBackend::GetImageMemoryRequirements(VkDevice device, VkImage image)
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);
	return memoryRequirements;
}

VkMemoryRequirements VulkanBackend::GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
	return memoryRequirements;
}

void* VulkanBackend::MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags)
{
	void* output;
	VulkanCheck(vkMapMemory(device, memory, offset, size, flags, &output));
	return output;
}

void VulkanBackend::UnmapMemory(VkDevice device, VkDeviceMemory memory)
{
	vkUnmapMemory(device, memory);
}

void VulkanBackend::FlushMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size)
{
	VkMappedMemoryRange mappedMemoryRange{};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = offset;
	mappedMemoryRange.size = size;
	
	VulkanCheck(vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange));
}

void VulkanBackend::InvalidateMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size)
{
	VkMappedMemoryRange mappedMemoryRange{};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = offset;
	mappedMemoryRange.size = size;
	
	VulkanCheck(vkInvalidateMappedMemoryRanges(device, 1, &mappedMemoryRange));
}

void VulkanBackend::BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize offset)
{
	VulkanCheck(vkBindImageMemory(device, image, memory, offset));
}
