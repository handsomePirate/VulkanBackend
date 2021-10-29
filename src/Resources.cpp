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
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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
	VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspect,
	VkAccessFlags sourceAccessMask, VkAccessFlags destinationAccessMask,
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

	barrier.srcAccessMask = sourceAccessMask;
	barrier.dstAccessMask = destinationAccessMask;

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanBackend::GenerateMips(const BackendData& backendData, VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat,
	int width, int height, int mipLevels)
{
	// Check if image format supports linear blitting.
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(backendData.physicalDevice, imageFormat, &formatProperties);

	if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0)
	{
		CoreLogError(VulkanLogger, "Vulkan backend: Texture format is not supported for linear blitting.");
	}
		

	// The barriers are used for transitions between image layouts.
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	int mipWidth = width;
	int mipHeight = height;

	// We iteratively blit/downsize the image subresources here to form the mip maps.
	for (uint32_t mip = 1; mip < (uint32_t)mipLevels; ++mip)
	{
		// Thanks to this barrier specification we are accessing the different mip subresources of
		// the image separately and also waiting for the previous operation on this image to finish
		// (whether it is the previous level generation or the buffer copy to GPU).
		barrier.subresourceRange.baseMipLevel = mip - 1;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		// We prepare the downsizing blit.
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = mip - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = mip;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// Downsize. The previous mip level is in the transfer source layout as it has been transitioned
		// by the previous pipeline barrier call, however the current mip level that we are down-sizing to
		// is still in the transfer destination layout from the complete transition before the generate mip
		// maps call.
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);
		// If the linear filter turns out to not be as precise as we want it to be, we might want to switch
		// to cubic.

		// Prepare for the next iteration.
		if (mipWidth > 1)
		{
			mipWidth >>= 1;
		}
		if (mipHeight > 1)
		{
			mipHeight >>= 1;
		}
	}

	// The last mip subresource also needs to be transitioned, so that the whole image resource then can be
	// transitioned as a whole to shader read optimal layout.
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanBackend::ReleaseImageOwnership(const BackendData& backendData, VkCommandBuffer commandBuffer, VkImage image, int mipLevels,
	VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspect, VkImageLayout layout,
	VkAccessFlags sourceAccessMask, int sourceQueueFamily, int destinationQueueFamily)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = sourceQueueFamily;
	barrier.dstQueueFamilyIndex = destinationQueueFamily;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.oldLayout = layout;
	barrier.newLayout = layout;
	barrier.srcAccessMask = sourceAccessMask;
	barrier.dstAccessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

void VulkanBackend::AcquireImageOwnership(const BackendData& backendData, VkCommandBuffer commandBuffer, VkImage image, int mipLevels,
	VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspect, VkImageLayout layout,
	VkAccessFlags destinationAccessMask, int sourceQueueFamily, int destinationQueueFamily)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = sourceQueueFamily;
	barrier.dstQueueFamilyIndex = destinationQueueFamily;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.oldLayout = layout;
	barrier.newLayout = layout;
	barrier.srcAccessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;
	barrier.dstAccessMask = destinationAccessMask;

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

VkImageView VulkanBackend::CreateImageView2D(const BackendData& backendData, VkImage image, VkFormat format, VkImageSubresourceRange& subresource)
{
	VkImageViewCreateInfo imageViewCreateInfo{};
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

void VulkanBackend::CopyBufferToBuffer(const BackendData& backendData, VkBuffer source, VkBuffer destination, VkDeviceSize size,
	VkCommandBuffer commandBuffer, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset)
{
	VkBufferCopy bufferCopy{};
	bufferCopy.size = size;
	bufferCopy.srcOffset = sourceOffset;
	bufferCopy.dstOffset = destinationOffset;
	vkCmdCopyBuffer(commandBuffer, source, destination, 1, &bufferCopy);
}

void VulkanBackend::CopyBufferToImage(const BackendData& backendData, VkBuffer source, VkImage destination, VkImageLayout layout,
	VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkImageAspectFlags aspect)
{
	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.imageSubresource.aspectMask = aspect;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;
	vkCmdCopyBufferToImage(commandBuffer, source, destination, layout, 1, &bufferImageCopy);
}

void VulkanBackend::CopyImageToBuffer(const BackendData& backendData, VkImage source, VkBuffer destination, VkImageLayout layout,
	VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkImageAspectFlags aspect, int32_t xOffset, int32_t yOffset)
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
