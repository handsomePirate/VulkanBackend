#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkDescriptorPool VulkanBackend::CreateDescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.maxSets = maxSets;

	VkDescriptorPool descriptorPool;
	VulkanCheck(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
	return descriptorPool;
}

void VulkanBackend::DestroyDescriptorPool(VkDevice device, VkDescriptorPool& descriptorPool)
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	descriptorPool = VK_NULL_HANDLE;
}

VkDescriptorSetLayout VulkanBackend::CreateDescriptorSetLayout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = (uint32_t)layoutBindings.size();
	descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	VulkanCheck(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
	return descriptorSetLayout;
}

void VulkanBackend::DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout& descriptorSetLayout)
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	descriptorSetLayout = VK_NULL_HANDLE;
}

VkDescriptorSet VulkanBackend::AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	VkDescriptorSet descriptorSet;
	VulkanCheck(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));
	return descriptorSet;
}

void VulkanBackend::FreeDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSet& descriptorSet)
{
	vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
	descriptorSet = VK_NULL_HANDLE;
}

VkShaderModule VulkanBackend::CreateShaderModule(VkDevice device, const std::vector<uint32_t>& data)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = (uint32_t)data.size();
	shaderModuleCreateInfo.pCode = data.data();

	VkShaderModule shaderModule;
	VulkanCheck(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule));
	return shaderModule;
}

void VulkanBackend::DestroyShaderModule(VkDevice device, VkShaderModule& shaderModule)
{
	vkDestroyShaderModule(device, shaderModule, nullptr);
	shaderModule = VK_NULL_HANDLE;
}

void VulkanBackend::WriteDescriptorSets(VkDevice device, VkDescriptorSet descriptorSet,
	const std::vector<VkDescriptorImageInfo>& imageDescriptors,
	const std::vector<VkDescriptorImageInfo>& storageBufferDescriptors,
	const std::vector<VkDescriptorImageInfo>& uniformBufferDescriptors)
{
	const bool hasImages = imageDescriptors.size() > 0;
	const bool hasStorageBuffers = storageBufferDescriptors.size() > 0;
	const bool hasUniformBuffers = uniformBufferDescriptors.size() > 0;
	const int setCount = (hasImages ? 1 : 0) + (hasStorageBuffers ? 1 : 0) + (hasUniformBuffers ? 1 : 0);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	int currentSet = 0;
	if (hasImages)
	{
		writeDescriptorSets.emplace_back();
		writeDescriptorSets[currentSet].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[currentSet].dstSet = descriptorSet;
		writeDescriptorSets[currentSet].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writeDescriptorSets[currentSet].dstBinding = 0;
		writeDescriptorSets[currentSet].pImageInfo = imageDescriptors.data();
		writeDescriptorSets[currentSet].descriptorCount = (uint32_t)imageDescriptors.size();
		++currentSet;
	}
	if (hasStorageBuffers)
	{
		writeDescriptorSets.emplace_back();
		writeDescriptorSets[currentSet].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[currentSet].dstSet = descriptorSet;
		writeDescriptorSets[currentSet].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeDescriptorSets[currentSet].dstBinding = (uint32_t)imageDescriptors.size();
		writeDescriptorSets[currentSet].pImageInfo = storageBufferDescriptors.data();
		writeDescriptorSets[currentSet].descriptorCount = (uint32_t)storageBufferDescriptors.size();
		++currentSet;
	}
	if (hasUniformBuffers)
	{
		writeDescriptorSets.emplace_back();
		writeDescriptorSets[currentSet].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[currentSet].dstSet = descriptorSet;
		writeDescriptorSets[currentSet].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[currentSet].dstBinding = (uint32_t)(imageDescriptors.size() + storageBufferDescriptors.size());
		writeDescriptorSets[currentSet].pImageInfo = uniformBufferDescriptors.data();
		writeDescriptorSets[currentSet].descriptorCount = (uint32_t)uniformBufferDescriptors.size();
		++currentSet;
	}

	vkUpdateDescriptorSets(device, setCount, writeDescriptorSets.data(), 0, nullptr);
}
