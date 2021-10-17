#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkImage VulkanBackend::CreateImage2D(VkDevice device, uint32_t width, uint32_t height, uint32_t layerCount, uint32_t mipCount,
	VkImageUsageFlags usage, VkFormat format, VkImageTiling imageTiling, VkSampleCountFlagBits samples)
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

	VkImage image;
	VulkanCheck(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

	return image;
}

void VulkanBackend::DestroyImage(VkDevice device, VkImage& image)
{
	vkDestroyImage(device, image, nullptr);
	image = VK_NULL_HANDLE;
}

VkImageView VulkanBackend::CreateImageView2D(VkDevice device, VkImage image, VkFormat format, VkImageSubresourceRange subresource)
{
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.subresourceRange = subresource;

	VkImageView imageView;
	VulkanCheck(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView));

	return imageView;
}

void VulkanBackend::DestroyImageView(VkDevice device, VkImageView& imageView)
{
	vkDestroyImageView(device, imageView, nullptr);
	imageView = VK_NULL_HANDLE;
}

VkSampler VulkanBackend::CreateImageSampler(VkDevice device, VkFilter magnificationFilter, VkFilter minificationFilter, VkBorderColor borderColor,
	VkSamplerAddressMode uAddressMode, VkSamplerAddressMode vAddressMode, VkSamplerAddressMode wAddressMode, VkSamplerMipmapMode mipmapMode, float maxAnisotropy)
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

	VkSampler sampler;
	VulkanCheck(vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler));

	return sampler;
}

void VulkanBackend::DestroyImageSampler(VkDevice device, VkSampler& sampler)
{
	vkDestroySampler(device, sampler, nullptr);
	sampler = VK_NULL_HANDLE;
}

VkBuffer VulkanBackend::CreateBuffer(VkDevice device, VkBufferUsageFlags usage, VkDeviceSize size)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.size = size;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;
	VulkanCheck(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));
	return buffer;
}

void VulkanBackend::DestroyBuffer(VkDevice device, VkBuffer& buffer)
{
	vkDestroyBuffer(device, buffer, nullptr);
	buffer = VK_NULL_HANDLE;
}

void VulkanBackend::CopyBufferToBuffer(VkDevice device, VkBuffer source, VkBuffer destination, VkDeviceSize size, VkCommandBuffer commandBuffer,
	VkQueue queue, VkDeviceSize sourceOffset, VkDeviceSize destinationOffset)
{
	VkBufferCopy bufferCopy{};
	bufferCopy.size = size;
	bufferCopy.srcOffset = sourceOffset;
	bufferCopy.dstOffset = destinationOffset;
	vkCmdCopyBuffer(commandBuffer, source, destination, 1, &bufferCopy);
}

void VulkanBackend::CopyBufferToImage(VkDevice device, VkBuffer source, VkImage destination, VkImageLayout layout, VkCommandBuffer commandBuffer,
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

void VulkanBackend::CopyImageToBuffer(VkDevice device, VkImage source, VkBuffer destination, VkImageLayout layout, VkCommandBuffer commandBuffer,
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
