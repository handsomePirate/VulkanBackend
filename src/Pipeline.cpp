#include "VulkanBackend/VulkanBackendAPI.hpp"
#include "VulkanBackend/ErrorCheck.hpp"

VkRenderPass VulkanBackend::CreateRenderPass(VkDevice device, const SurfaceData& surfaceData)
{
	const int attachmentCount = 1;
	VkAttachmentDescription attachments[attachmentCount];
	// Color attachment
	attachments[0] = {};
	attachments[0].format = surfaceData.surfaceFormat.format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	//attachments[1] = {};
	//attachments[1].format = surfaceData.depthFormat;
	//attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	//attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference{};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference{};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	//subpassDescription.pDepthStencilAttachment = &depthReference;

	// Subpass dependencies for layout transitions
	const int dependencyCount = 2;
	VkSubpassDependency dependencies[dependencyCount];

	dependencies[0] = {};
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1] = {};
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = dependencyCount;
	renderPassInfo.pDependencies = dependencies;

	VkRenderPass renderPass;
	VulkanCheck(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
	return renderPass;
}

void VulkanBackend::DestroyRenderPass(VkDevice device, VkRenderPass& renderPass)
{
	vkDestroyRenderPass(device, renderPass, nullptr);
	renderPass = VK_NULL_HANDLE;
}

VkPipelineCache VulkanBackend::CreatePipelineCache(VkDevice device)
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VkPipelineCache pipelineCache;
	VulkanCheck(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
	return pipelineCache;
}

void VulkanBackend::DestroyPipelineCache(VkDevice device, VkPipelineCache& pipelineCache)
{
	vkDestroyPipelineCache(device, pipelineCache, nullptr);
	pipelineCache = VK_NULL_HANDLE;
}

VkPipelineLayout VulkanBackend::CreatePipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkPushConstantRange pushConstantRange)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	VkPipelineLayout pipelineLayout;
	VulkanCheck(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
	return pipelineLayout;
}

void VulkanBackend::DestroyPipelineLayout(VkDevice device, VkPipelineLayout& pipelineLayout)
{
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	pipelineLayout = VK_NULL_HANDLE;
}

VkPipeline VulkanBackend::CreateGraphicsPipeline(VkDevice device, VkPrimitiveTopology primitiveTopology, VkPolygonMode polygonMode,
	VkCullModeFlags cullMode, VkFrontFace frontFace, VkColorComponentFlags colorComponents, VkBool32 depthTestEnable,
	VkBool32 depthWriteEnable, VkCompareOp compareOp, VkSampleCountFlagBits sampleCount, const std::vector<VkDynamicState>& dynamicStates,
	VkPipelineVertexInputStateCreateInfo& vertexInputState, VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
	const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages, VkPipelineCache pipelineCache)
{
	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo{};
	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
	assemblyCreateInfo.topology = primitiveTopology;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.polygonMode = polygonMode;
	rasterizationStateCreateInfo.cullMode = cullMode;
	rasterizationStateCreateInfo.frontFace = frontFace;
	rasterizationStateCreateInfo.flags = 0;
	rasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentCreateInfo{};
	colorBlendAttachmentCreateInfo.colorWriteMask = colorComponents;
	colorBlendAttachmentCreateInfo.blendEnable = VK_TRUE;
	colorBlendAttachmentCreateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentCreateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentCreateInfo.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentCreateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentCreateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentCreateInfo.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.pAttachments = &colorBlendAttachmentCreateInfo;
	colorBlendCreateInfo.attachmentCount = 1;

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = sampleCount;
	pipelineMultisampleStateCreateInfo.flags = 0;
	pipelineMultisampleStateCreateInfo.minSampleShading = 0.3f;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.flags = 0;

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = depthTestEnable;
	depthStencilCreateInfo.depthWriteEnable = depthWriteEnable;
	depthStencilCreateInfo.depthCompareOp = compareOp;

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &assemblyCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.renderPass = renderPass;

	VkPipeline pipeline;
	VulkanCheck(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
	return pipeline;
}

VkPipeline VulkanBackend::CreateComputePipeline(VkDevice device, VkPipelineLayout pipelineLayout, VkPipelineShaderStageCreateInfo& shaderStage,
	VkPipelineCache pipelineCache)
{
	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = pipelineLayout;
	computePipelineCreateInfo.stage = shaderStage;

	VkPipeline pipeline;
	VulkanCheck(vkCreateComputePipelines(device, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
	return pipeline;
}

void VulkanBackend::DestroyPipeline(VkDevice device, VkPipeline& pipeline)
{
	vkDestroyPipeline(device, pipeline, nullptr);
	pipeline = VK_NULL_HANDLE;
}
