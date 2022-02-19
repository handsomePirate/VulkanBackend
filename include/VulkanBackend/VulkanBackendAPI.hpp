#pragma once
#include <SoftwareCore/Logger.hpp>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <vector>

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
		VmaAllocator allocator;
		VkCommandPool transferCommandPool;
		VkCommandPool generalCommandPool;
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

	void CreateSurface(const BackendData& backendData, SurfaceData& surfaceData, void* windowHandle, void* connection);
	void DestroySurface(const BackendData& backendData, VkSurfaceKHR& surface);

	void GetDepthFormat(const BackendData& backendData, SurfaceData& surfaceData);
	void GetSurfaceFormat(const BackendData& backendData, SurfaceData& surfaceData);
	void GetSurfaceCapabilities(const BackendData& backendData, SurfaceData& surfaceData);
	void GetSurfaceExtent(const BackendData& backendData, SurfaceData& surfaceData);
	void GetPresentMode(const BackendData& backendData, SurfaceData& surfaceData, bool vSync = false);
	void GetSwapchainImageCount(SurfaceData& surfaceData);

	VkPhysicalDeviceMemoryProperties GetDeviceMemoryProperties(VkPhysicalDevice device);

	void FilterPresentQueues(const BackendData& backendData, SurfaceData& surfaceData);
	// These (Select functions) should be used only after the present queue candidates have been filtered.
	void SelectPresentQueue(const BackendData& backendData, SurfaceData& surfaceData);
	void SelectPresentComputeQueue(const BackendData& backendData, SurfaceData& surfaceData);

	// ======================== Commands =======================

	VkCommandPool CreateCommandPool(const BackendData& backendData, uint32_t queueIndex, VkCommandPoolCreateFlags flags = 0);
	void DestroyCommandPool(const BackendData& backendData, VkCommandPool& commandPool);

	VkCommandBuffer AllocateCommandBuffer(const BackendData& backendData, VkCommandPool commandPool,
		VkCommandBufferLevel commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void FreeCommandBuffer(const BackendData& backendData, VkCommandPool commandPool, VkCommandBuffer& commandBuffer);

	void AllocateCommandBuffers(const BackendData& backendData, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount,
		VkCommandBufferLevel commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void FreeCommandBuffers(const BackendData& backendData, VkCommandPool commandPool, VkCommandBuffer* commandBuffers, uint32_t bufferCount);

	void ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags = 0);
	void ResetCommandPool(const BackendData& backendData, VkCommandPool commandPool, VkCommandPoolResetFlags flags = 0);

	// ==================== Synchronization ====================

	VkSemaphore CreateSemaphore(const BackendData& backendData);
	void DestroySemaphore(const BackendData& backendData, VkSemaphore& semaphore);

	VkFence CreateFence(const BackendData& backendData, VkFenceCreateFlags flags = 0);
	void DestroyFence(const BackendData& backendData, VkFence& fence);

	// ======================= Resources =======================
	
	struct Image
	{
		VkImage image = nullptr;
		VmaAllocation allocation = nullptr;
	};

	Image CreateImage2D(const BackendData& backendData, uint32_t width, uint32_t height, uint32_t layerCount, uint32_t mipCount, VkImageUsageFlags usage,
		VkFormat format, VmaMemoryUsage residency, VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	void DestroyImage(const BackendData& backendData, Image& image);

	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout currentLayout, VkImageLayout nextLayout, VkImage image, uint32_t mipLevels,
		VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspect,
		VkAccessFlags sourceAccessMask, VkAccessFlags destinationAccessMask,
		uint32_t sourceQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, uint32_t destinationQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED);
	void GenerateMips(const BackendData& backendData, VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, int width, int height, int mipLevels);
	
	void ReleaseImageOwnership(const BackendData& backendData, VkCommandBuffer commandBuffer, VkImage image, int mipLevels,
		VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspect, VkImageLayout layout,
		VkAccessFlags sourceAccessMask, int sourceQueueFamily, int destinationQueueFamily);
	void AcquireImageOwnership(const BackendData& backendData, VkCommandBuffer commandBuffer, VkImage image, int mipLevels,
		VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspect, VkImageLayout layout,
		VkAccessFlags destinationAccessMask, int sourceQueueFamily, int destinationQueueFamily);

	VkImageView CreateImageView2D(const BackendData& backendData, VkImage image, VkFormat format, VkImageSubresourceRange& subresource);
	void DestroyImageView(const BackendData& backendData, VkImageView& imageView);

	VkSampler CreateImageSampler(const BackendData& backendData, VkFilter magnificationFilter, VkFilter minificationFilter, VkBorderColor borderColor,
		VkSamplerAddressMode uAddressMode, VkSamplerAddressMode vAddressMode, VkSamplerAddressMode wAddressMode,
		float minLod, float maxLod, float mipLodBias = 0.f, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, float maxAnisotropy = 1.f);
	void DestroyImageSampler(const BackendData& backendData, VkSampler& sampler);

	struct Buffer
	{
		VkBuffer buffer = nullptr;
		VmaAllocation allocation = nullptr;
	};

	Buffer CreateBuffer(const BackendData& backendData, VkBufferUsageFlags usage, VkDeviceSize size, VmaMemoryUsage residency);
	void DestroyBuffer(const BackendData& backendData, Buffer& buffer);

	void ReleaseBufferOwnership(const BackendData& backendData, VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize size,
		VkDeviceSize offset, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkAccessFlags sourceAccessMask,
		int sourceQueueFamily, int destinationQueueFamily);
	void AcquireBufferOwnership(const BackendData& backendData, VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize size,
		VkDeviceSize offset, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkAccessFlags destinationAccessMask,
		int sourceQueueFamily, int destinationQueueFamily);

	void CopyBufferToBuffer(const BackendData& backendData, VkBuffer source, VkBuffer destination, VkDeviceSize size,
		VkCommandBuffer commandBuffer, VkDeviceSize sourceOffset = 0, VkDeviceSize destinationOffset = 0);
	void CopyBufferToImage(const BackendData& backendData, VkBuffer source, VkImage destination, VkImageLayout layout,
		VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkImageAspectFlags aspect);
	void CopyImageToBuffer(const BackendData& backendData, VkImage source, VkBuffer destination, VkImageLayout layout,
		VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkImageAspectFlags aspect, int32_t xOffset = 0, int32_t yOffset = 0);

	// ====================== Presentation =====================

	VkSwapchainKHR CreateSwapchain(const BackendData& backendData, const SurfaceData& surfaceData,
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
	VkSwapchainKHR RecreateSwapchain(const BackendData& backendData, const SurfaceData& surfaceData,
		VkSwapchainKHR& oldSwapchain, VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
	void DestroySwapchain(const BackendData& backendData, VkSwapchainKHR& swapchain);

	void GetSwapchainImages(const BackendData& backendData, VkSwapchainKHR swapchain, std::vector<VkImage>& images);

	VkFramebuffer CreateFramebuffer(const BackendData& backendData, uint32_t width, uint32_t height, VkRenderPass renderPass,
		const std::vector<VkImageView>& attachments);
	void DestroyFramebuffer(const BackendData& backendData, VkFramebuffer& framebuffer);

	// ======================== Pipeline =======================

	VkRenderPass CreateRenderPass(const BackendData& backendData, const SurfaceData& surfaceData, bool depth = false,
		VkImageLayout colorInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout colorFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	void DestroyRenderPass(const BackendData& backendData, VkRenderPass& renderPass);

	VkPipelineCache CreatePipelineCache(const BackendData& backendData);
	void DestroyPipelineCache(const BackendData& backendData, VkPipelineCache& pipelineCache);

	VkPipelineLayout CreatePipelineLayout(const BackendData& backendData, VkDescriptorSetLayout descriptorSetLayout, VkPushConstantRange pushConstantRange);
	void DestroyPipelineLayout(const BackendData& backendData, VkPipelineLayout& pipelineLayout);

	VkPipeline CreateGraphicsPipeline(const BackendData& backendData, VkPrimitiveTopology primitiveTopology, VkPolygonMode polygonMode,
		VkCullModeFlags cullMode, VkFrontFace frontFace, VkColorComponentFlags colorComponents, VkBool32 depthTestEnable,
		VkBool32 depthWriteEnable, VkCompareOp compareOp, VkSampleCountFlagBits sampleCount, const std::vector<VkDynamicState>& dynamicStates,
		VkPipelineVertexInputStateCreateInfo& vertexInputState, VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, VkPipelineCache pipelineCache = VK_NULL_HANDLE);
	VkPipeline CreateComputePipeline(const BackendData& backendData, VkPipelineLayout pipelineLayout,
		const VkPipelineShaderStageCreateInfo& shaderStage, VkPipelineCache pipelineCache);
	void DestroyPipeline(const BackendData& backendData, VkPipeline& pipeline);

	// ========================= Shader ========================

	VkDescriptorPool CreateDescriptorPool(const BackendData& backendData, const std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets);
	void DestroyDescriptorPool(const BackendData& backendData, VkDescriptorPool& descriptorPool);
	
	VkDescriptorSetLayout CreateDescriptorSetLayout(const BackendData& backendData, const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings);
	void DestroyDescriptorSetLayout(const BackendData& backendData, VkDescriptorSetLayout& descriptorSetLayout);
	
	VkDescriptorSet AllocateDescriptorSet(const BackendData& backendData, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
	void FreeDescriptorSet(const BackendData& backendData, VkDescriptorPool descriptorPool, VkDescriptorSet& descriptorSet);
	
	VkShaderModule CreateShaderModule(const BackendData& backendData, const std::vector<uint32_t>& bytes);
	void DestroyShaderModule(const BackendData& backendData, VkShaderModule& shaderModule);

	void WriteDescriptorSets(const BackendData& backendData, VkDescriptorSet descriptorSet,
		const std::vector<VkDescriptorImageInfo>& imageDescriptors,
		const std::vector<VkDescriptorImageInfo>& storageBufferDescriptors,
		const std::vector<VkDescriptorImageInfo>& uniformBufferDescriptors);
}
