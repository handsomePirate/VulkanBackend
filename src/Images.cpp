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
