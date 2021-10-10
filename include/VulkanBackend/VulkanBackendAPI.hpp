#pragma once
#include "Logger.hpp"
#include <SoftwareCore/Logger.hpp>
#include <vulkan/vulkan_core.h>

namespace VulkanBackend
{
	struct Initialized
	{
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		std::vector<VkQueue> generalQueues;
		int generalFamilyIndex;
		std::vector<VkQueue> computeQueues;
		int computeFamilyIndex;
		std::vector<VkQueue> transferQueues;
		int transferFamilyIndex;
		std::vector<VkQueue> presentQueueCandidates;
	};

	// ======================== Backend ========================

	Initialized Initialize(const char* configFilePath);
	void Shutdown(Initialized& initialized);

	VkSurfaceKHR CreateSurface(VkInstance instance, void* windowHandle, void* connection);

	VkFormat GetDepthFormat(VkPhysicalDevice device);
	VkSurfaceFormatKHR GetSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkPhysicalDeviceMemoryProperties GetDeviceMemoryProperties(VkPhysicalDevice device);

	void FilterPresentQueues(Initialized& initialized, VkSurfaceKHR surface);
	// These (Select functions) should be used only after the present queue candidates have been filtered.
	VkQueue SelectPresentQueue(const Initialized& initialized);
	VkQueue SelectPresentComputeQueue(const Initialized& initialized);

	// ======================== Commands =======================

	VkCommandPool CreateCommandPool(VkDevice device, uint32_t queueIndex, VkCommandPoolCreateFlags flags = 0);
	void DestroyCommandPool(VkDevice device, VkCommandPool& commandPool);

	VkCommandBuffer AllocateCommandBuffer(VkDevice device, VkCommandPool commandPool,
		VkCommandBufferLevel commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void FreeCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer& commandBuffer);

	void AllocateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount,
		VkCommandBufferLevel commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount);

	void ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags = 0);
	void ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags = 0);

	// ==================== Synchronization ====================

	VkSemaphore CreateSemaphore(VkDevice device);
	void DestroySemaphore(VkDevice device, VkSemaphore& semaphore);

	VkFence CreateFence(VkDevice device, VkFenceCreateFlags flags = 0);
	void DestroyFence(VkDevice device, VkFence& fence);

	// ========================= Memory ========================

	VkDeviceMemory AllocateMemory(VkDevice device, VkDeviceSize size, uint32_t memoryTypeIndex,
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties, VkMemoryPropertyFlags memoryProperties);
	void FreeMemory(VkDevice device, VkDeviceMemory& memory);

	VkMemoryRequirements GetImageMemoryRequirements(VkDevice device, VkImage image);
	VkMemoryRequirements GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer);

	void* MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags = 0);
	void UnmapMemory(VkDevice device, VkDeviceMemory memory);
	void FlushMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size);
	void InvalidateMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size);

	void BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize offset = 0);

	// ========================= Images =========================
	
	VkImage CreateImage2D(VkDevice device, uint32_t width, uint32_t height, uint32_t layerCount, uint32_t mipCount, VkImageUsageFlags usage,
		VkFormat format, VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	void DestroyImage(VkDevice device, VkImage& image);

	VkImageView CreateImageView2D(VkDevice device, VkImage image, VkFormat format, VkImageSubresourceRange subresource);
	void DestroyImageView(VkDevice device, VkImageView& imageView);

	VkSampler CreateImageSampler(VkDevice device, VkFilter magnificationFilter, VkFilter minificationFilter, VkBorderColor borderColor,
		VkSamplerAddressMode uAddressMode, VkSamplerAddressMode vAddressMode, VkSamplerAddressMode wAddressMode,
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, float maxAnisotropy = 1.f);
	void DestroyImageSampler(VkDevice device, VkSampler& sampler);
}
