#pragma once
#include "Logger.hpp"
#include <SoftwareCore/Logger.hpp>
#include <vulkan/vulkan_core.h>

namespace VulkanBackend
{
	// ======================== Backend ========================

	struct BackendData
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

	BackendData Initialize(const char* configFilePath);
	void Shutdown(BackendData& backendData);

	// ======================== Surface ========================

	struct SurfaceData
	{
		VkSurfaceKHR surface;
		std::vector<VkQueue> presentQueues;
		VkQueue defaultPresentQueue;
		VkQueue computePresentQueue;
		VkFormat depthFormat;
		VkSurfaceFormatKHR surfaceFormat;
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		VkSurfaceTransformFlagBitsKHR surfaceTransform;
		VkCompositeAlphaFlagBitsKHR compositeAlpha;
		VkPresentModeKHR presentMode;
		VkExtent2D surfaceExtent;
		uint32_t width;
		uint32_t height;
		uint32_t swapchainImageCount;
	};

	void CreateSurface(VkInstance instance, SurfaceData& surfaceData, void* windowHandle, void* connection);

	void GetDepthFormat(VkPhysicalDevice device, SurfaceData& surfaceData);
	void GetSurfaceFormat(VkPhysicalDevice device, SurfaceData& surfaceData);
	void GetSurfaceCapabilities(VkPhysicalDevice device, SurfaceData& surfaceData);
	void GetSurfaceExtent(VkPhysicalDevice device, SurfaceData& surfaceData);
	void GetPresentMode(VkPhysicalDevice device, SurfaceData& surfaceData, bool vSync = false);
	void GetSwapchainImageCount(SurfaceData& surfaceData);

	VkPhysicalDeviceMemoryProperties GetDeviceMemoryProperties(VkPhysicalDevice device);

	void FilterPresentQueues(const BackendData& backendData, SurfaceData& surfaceData);
	// These (Select functions) should be used only after the present queue candidates have been filtered.
	void SelectPresentQueue(const BackendData& backendData, SurfaceData& surfaceData);
	void SelectPresentComputeQueue(const BackendData& backendData, SurfaceData& surfaceData);

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

	// ======================= Resources =======================
	
	VkImage CreateImage2D(VkDevice device, uint32_t width, uint32_t height, uint32_t layerCount, uint32_t mipCount, VkImageUsageFlags usage,
		VkFormat format, VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	void DestroyImage(VkDevice device, VkImage& image);

	VkImageView CreateImageView2D(VkDevice device, VkImage image, VkFormat format, VkImageSubresourceRange subresource);
	void DestroyImageView(VkDevice device, VkImageView& imageView);

	VkSampler CreateImageSampler(VkDevice device, VkFilter magnificationFilter, VkFilter minificationFilter, VkBorderColor borderColor,
		VkSamplerAddressMode uAddressMode, VkSamplerAddressMode vAddressMode, VkSamplerAddressMode wAddressMode,
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, float maxAnisotropy = 1.f);
	void DestroyImageSampler(VkDevice device, VkSampler& sampler);

	VkBuffer CreateBuffer(VkDevice device, VkBufferUsageFlags usage, VkDeviceSize size);
	void DestroyBuffer(VkDevice device, VkBuffer& buffer);

	void CopyBufferToBuffer(VkDevice device, VkBuffer source, VkBuffer destination, VkDeviceSize size, VkCommandBuffer commandBuffer,
		VkQueue queue, VkDeviceSize sourceOffset = 0, VkDeviceSize destinationOffset = 0);
	void CopyBufferToImage(VkDevice device, VkBuffer source, VkImage destination, VkImageLayout layout, VkCommandBuffer commandBuffer,
		VkQueue queue, uint32_t width, uint32_t height, VkImageAspectFlags aspect);
	void CopyImageToBuffer(VkDevice device, VkImage source, VkBuffer destination, VkImageLayout layout, VkCommandBuffer commandBuffer,
		VkQueue queue, uint32_t width, uint32_t height, VkImageAspectFlags aspect, int32_t xOffset = 0, int32_t yOffset = 0);

	// ====================== Presentation =====================

	VkSwapchainKHR CreateSwapchain(const BackendData& backendData, uint32_t width, uint32_t height, const SurfaceData& surfaceData,
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
	VkSwapchainKHR RecreateSwapchain(const BackendData& backendData, uint32_t width, uint32_t height, const SurfaceData& surfaceData,
		VkSwapchainKHR& oldSwapchain, VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
	void DestroySwapchain(VkDevice device, VkSwapchainKHR& swapchain);

	VkFramebuffer CreateFramebuffer(VkDevice device, uint32_t width, uint32_t height, VkRenderPass renderPass,
		const std::vector<VkImageView>& attachments);
	void DestroyFramebuffer(VkDevice device, VkFramebuffer& framebuffer);

	// ======================== Pipeline =======================

	VkRenderPass CreateRenderPass(VkDevice device, const SurfaceData& surfaceData);
	void DestroyRenderPass(VkDevice device, VkRenderPass& renderPass);

	VkPipelineCache CreatePipelineCache(VkDevice device);
	void DestroyPipelineCache(VkDevice device, VkPipelineCache& pipelineCache);

	VkPipelineLayout CreatePipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkPushConstantRange pushConstantRange);
	void DestroyPipelineLayout(VkDevice device, VkPipelineLayout& pipelineLayout);

	VkPipeline CreateGraphicsPipeline(VkDevice device, VkPrimitiveTopology primitiveTopology, VkPolygonMode polygonMode,
		VkCullModeFlags cullMode, VkFrontFace frontFace, VkColorComponentFlags colorComponents, VkBool32 depthTestEnable,
		VkBool32 depthWriteEnable, VkCompareOp compareOp, VkSampleCountFlagBits sampleCount, const std::vector<VkDynamicState>& dynamicStates,
		VkPipelineVertexInputStateCreateInfo& vertexInputState, VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	VkPipeline CreateComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkPipelineShaderStageCreateInfo& shaderStage, VkPipelineCache pipelineCache);
	void DestroyPipeline(VkDevice device, VkPipeline& pipeline);

	// ========================= Shader ========================

	VkDescriptorPool CreateDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets);
	void DestroyDescriptorPool(VkDevice device, VkDescriptorPool& descriptorPool);
	
	VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings);
	void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout& descriptorSetLayout);
	
	VkDescriptorSet AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
	void FreeDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSet& descriptorSet);
	
	VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& bytes);
	void DestroyShaderModule(VkDevice device, VkShaderModule& shaderModule);

	void WriteDescriptorSets(VkDevice device, VkDescriptorSet descriptorSet,
		const std::vector<VkDescriptorImageInfo>& imageDescriptors,
		const std::vector<VkDescriptorImageInfo>& storageBufferDescriptors,
		const std::vector<VkDescriptorImageInfo>& uniformBufferDescriptors);
}
