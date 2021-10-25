#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VulkanBackend::Image VulkanBackend::CreateImage2D(const BackendData& backendData, uint32_t width, uint32_t height, uint32_t layerCount, uint32_t mipCount,
	VkImageUsageFlags usage, VkFormat format, VmaMemoryUsage residency, VkImageTiling imageTiling, VkSampleCountFlagBits samples)
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = mipCount;
	imageCreateInfo.arrayLayers = layerCount;
	imageCreateInfo.samples = samples;
	imageCreateInfo.tiling = imageTiling;
	imageCreateInfo.usage = usage;

	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.usage = residency;

	Image image;
	VulkanCheck(vmaCreateImage(backendData.allocator, &imageCreateInfo, &allocationInfo, &image.image, &image.allocation, nullptr));

	return image;
}

void VulkanBackend::DestroyImage(const BackendData& backendData, VulkanBackend::Image& image)
{
	vmaDestroyImage(backendData.allocator, image.image, image.allocation);
	image.image = VK_NULL_HANDLE;
	image.allocation = VK_NULL_HANDLE;
}

void VulkanBackend::TransitionImageLayout(VkCommandBuffer commandBuffer,
	VkImageLayout currentLayout, VkImageLayout nextLayout, VkImage image, uint32_t mipLevels,
	uint32_t sourceQueueFamilyIndex, uint32_t destinationQueueFamilyIndex)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	// Undefined here means we do not care about the original contents of the image.
	barrier.oldLayout = currentLayout;
	barrier.newLayout = nextLayout;

	// NOTE: This can be used to transfer queue ownership.
	barrier.srcQueueFamilyIndex = sourceQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = destinationQueueFamilyIndex;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// To transition the image, a pipeline barrier is needed.
	if (currentLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		nextLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (
		currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		nextLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (
		currentLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
		nextLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (
		currentLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		nextLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		CoreLogError(VulkanLogger, "Vulkan backend: Unsupported layout transition.");
	}

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

VkImageView VulkanBackend::CreateImageView2D(const BackendData& backendData, VkImage image, VkFormat format, VkImageSubresourceRange subresource)
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.subresourceRange = subresource;

	VkImageView imageView;
	VulkanCheck(vkCreateImageView(backendData.logicalDevice, &imageViewCreateInfo, nullptr, &imageView));

	return imageView;
}

void VulkanBackend::DestroyImageView(const BackendData& backendData, VkImageView& imageView)
{
	vkDestroyImageView(backendData.logicalDevice, imageView, nullptr);
	imageView = VK_NULL_HANDLE;
}

VkSampler VulkanBackend::CreateImageSampler(const BackendData& backendData, VkFilter magnificationFilter,
	VkFilter minificationFilter, VkBorderColor borderColor,
	VkSamplerAddressMode uAddressMode, VkSamplerAddressMode vAddressMode, VkSamplerAddressMode wAddressMode,
	float minLod, float maxLod, float mipLodBias, VkSamplerMipmapMode mipmapMode, float maxAnisotropy)
{
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.maxAnisotropy = maxAnisotropy;
	samplerCreateInfo.magFilter = magnificationFilter;
	samplerCreateInfo.minFilter = minificationFilter;
	samplerCreateInfo.mipmapMode = mipmapMode;
	samplerCreateInfo.addressModeU = uAddressMode;
	samplerCreateInfo.addressModeV = vAddressMode;
	samplerCreateInfo.addressModeW = wAddressMode;
	samplerCreateInfo.borderColor = borderColor;
	samplerCreateInfo.minLod = minLod;
	samplerCreateInfo.maxLod = maxLod;
	samplerCreateInfo.mipLodBias = mipLodBias;

	VkSampler sampler;
	VulkanCheck(vkCreateSampler(backendData.logicalDevice, &samplerCreateInfo, nullptr, &sampler));

	return sampler;
}

void VulkanBackend::DestroyImageSampler(const BackendData& backendData, VkSampler& sampler)
{
	vkDestroySampler(backendData.logicalDevice, sampler, nullptr);
	sampler = VK_NULL_HANDLE;
}

VulkanBackend::Buffer VulkanBackend::CreateBuffer(const BackendData& backendData, VkBufferUsageFlags usage, VkDeviceSize size,
	VmaMemoryUsage residency)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.size = size;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.usage = residency;

	Buffer buffer;
	VulkanCheck(vmaCreateBuffer(backendData.allocator, &bufferCreateInfo, &allocationInfo, &buffer.buffer, &buffer.allocation, nullptr));

	return buffer;
}

void VulkanBackend::DestroyBuffer(const BackendData& backendData, VulkanBackend::Buffer& buffer)
{
	vmaDestroyBuffer(backendData.allocator, buffer.buffer, buffer.allocation);
	buffer.buffer = VK_NULL_HANDLE;
	buffer.allocation = VK_NULL_HANDLE;
}

void VulkanBackend::CopyBufferToBuffer(const BackendData& backendData, VkBuffer source, VkBuffer destination, VkDeviceSize size, VkCommandBuffer commandBuffer,
	VkQueue queue, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset)
{
	VkBufferCopy bufferCopy{};
	bufferCopy.size = size;
	bufferCopy.srcOffset = sourceOffset;
	bufferCopy.dstOffset = destinationOffset;
	vkCmdCopyBuffer(commandBuffer, source, destination, 1, &bufferCopy);
}

void VulkanBackend::CopyBufferToImage(const BackendData& backendData, VkBuffer source, VkImage destination, VkImageLayout layout, VkCommandBuffer commandBuffer,
	VkQueue queue, uint32_t width, uint32_t height, VkImageAspectFlags aspect)
{
	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.imageSubresource.aspectMask = aspect;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;
	vkCmdCopyBufferToImage(commandBuffer, source, destination, layout, 1, &bufferImageCopy);
}

void VulkanBackend::CopyImageToBuffer(const BackendData& backendData, VkImage source, VkBuffer destination, VkImageLayout layout, VkCommandBuffer commandBuffer,
	VkQueue queue, uint32_t width, uint32_t height, VkImageAspectFlags aspect, int32_t xOffset, int32_t yOffset)
{
	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.imageSubresource.aspectMask = aspect;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;
	bufferImageCopy.imageOffset = { xOffset, yOffset, 0 };
	vkCmdCopyImageToBuffer(commandBuffer, source, layout, destination, 1, &bufferImageCopy);
}
