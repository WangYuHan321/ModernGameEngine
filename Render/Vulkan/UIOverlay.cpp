#include "UIOverlay.h"

Render::Vulkan::UIOverlay::UIOverlay()
{

	ImGui::CreateContext();

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = scale;
}

Render::Vulkan::UIOverlay::~UIOverlay()
{
	if (ImGui::GetCurrentContext())
		ImGui::DestroyContext();
}

void Render::Vulkan::UIOverlay::PrepareResource()
{
	ImGuiIO& io = ImGui::GetIO();

	// Create font texture
	unsigned char* fontData;
	int texWidth, texHeight;
	const std::string filePath = "./Asset/font/PressStart2P-Regular.ttf";

#if defined(__ANDROID__)
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, "font/Roboto-Medium.ttf", AASSET_MODE_STREAMING);
    if (asset) {
        size_t size = AAsset_getLength(asset);
        assert(size > 0);
        char *fontAsset = new char[size];
        AAsset_read(asset, fontAsset, size);
        AAsset_close(asset);
        io.Fonts->AddFontFromMemoryTTF(fontAsset, size, 14.0f * scale);
        delete[] fontAsset;
    }
#else
    io.Fonts->AddFontFromFileTTF(filePath.c_str(), 16.0f * scale);
#endif

	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

	//SRS - Set ImGui style scale factor to handle retina and other HiDPI displays (same as font scaling above)
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);

	// Create target image for copy
	VkImageCreateInfo imageInfo = Render::Vulkan::Initializer::ImageCreateInfo();
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.extent.width = texWidth;
	imageInfo.extent.height = texHeight;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageInfo, nullptr, &fontImage));
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device->logicalDevice, fontImage, &memReqs);
	VkMemoryAllocateInfo memAllocInfo = Render::Vulkan::Initializer::MemoryAllocInfo();
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = device->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &fontMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, fontImage, fontMemory, 0));

	VkImageViewCreateInfo viewInfo = Render::Vulkan::Initializer::ImageViewCreateInfo();
	viewInfo.image = fontImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;
	VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &fontView));

	Render::Vulkan::Buffer stagingBuffer;

	VK_CHECK_RESULT(device->CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		uploadSize));

	stagingBuffer.Map();
	memcpy(stagingBuffer.mapped, fontData, uploadSize);
	stagingBuffer.UnMap();

	VkCommandBuffer copyCmd = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	Render::Vulkan::Tool::SetImageLayout(
		copyCmd,
		fontImage,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy bufferCopyRegion{};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = texWidth;
	bufferCopyRegion.imageExtent.height = texHeight;
	bufferCopyRegion.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer.buffer,
		fontImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferCopyRegion);

	Render::Vulkan::Tool::SetImageLayout(
		copyCmd,
		fontImage,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	);

	device->FlushCommandBuffer(copyCmd, queue, true);

	stagingBuffer.Destroy();

	VkSamplerCreateInfo samplerInfo = Render::Vulkan::Initializer::SamplerCreateInfo();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &sampler));

	std::vector<VkDescriptorPoolSize> poolSizes = {
	Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};
	VkDescriptorPoolCreateInfo descriptorPoolInfo = Render::Vulkan::Initializer::DescriptorPoolCreateInfo(poolSizes, 2);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool));

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = Render::Vulkan::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout));

	VkDescriptorSetAllocateInfo allocInfo = Render::Vulkan::Initializer::DescriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device->logicalDevice, &allocInfo, &descriptorSet));
	VkDescriptorImageInfo fontDescriptor = Render::Vulkan::Initializer::DescriptorImageInfo(
		sampler,
		fontView,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)
	};
	vkUpdateDescriptorSets(device->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void Render::Vulkan::UIOverlay::PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat)
{
    // Pipeline layout
    // Push constants for UI rendering parameters
    VkPushConstantRange pushConstantRange{ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(PushConstBlock) };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange
    };
    VK_CHECK_RESULT(vkCreatePipelineLayout(device->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

    // Setup graphics pipeline for UI rendering
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Render::Vulkan::Initializer::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState = Render::Vulkan::Initializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    // Enable blending
    VkPipelineColorBlendAttachmentState blendAttachmentState{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    // Vertex bindings an attributes based on ImGui vertex definition
    std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
            { .binding = 0, .stride = sizeof(ImDrawVert), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
    };
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
            { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(ImDrawVert, pos) },
            { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(ImDrawVert, uv) },
            { .location = 2, .binding = 0, .format = VK_FORMAT_R8G8B8A8_UNORM, .offset = offsetof(ImDrawVert, col) },
    };
    VkPipelineVertexInputStateCreateInfo vertexInputState = Render::Vulkan::Initializer::PipelineVertexInputStateCreateInfo();
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

    VkPipelineColorBlendStateCreateInfo colorBlendState = Render::Vulkan::Initializer::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = Render::Vulkan::Initializer::PipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);
    VkPipelineViewportStateCreateInfo viewportState = Render::Vulkan::Initializer::PipelineViewportStateCreateInfo(1, 1, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = Render::Vulkan::Initializer::PipelineMultisampleStateCreateInfo(rasterizationSamples);
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = Render::Vulkan::Initializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shaders.size()),
            .pStages = shaders.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationState,
            .pMultisampleState = &multisampleState,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicState,
            .layout = pipelineLayout,
            .renderPass = renderPass,
            .subpass = subPass,
    };
#if defined(VK_KHR_dynamic_rendering)
    // If we are using dynamic rendering (renderPass is null), we must define color, depth and stencil attachments at pipeline create time
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    if (renderPass == VK_NULL_HANDLE) {
        pipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &colorFormat,
                .depthAttachmentFormat = depthFormat,
                .stencilAttachmentFormat = depthFormat
        };
        pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
    }
#endif
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

bool Render::Vulkan::UIOverlay::Update()
{
	ImDrawData* imDrawData = ImGui::GetDrawData();
	bool updateCmdBuffers = false;

	if (!imDrawData) { return false; };

	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// Update buffers only if vertex or index count has been changed compared to current buffer size
	if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
		return false;
	}

	// Vertex buffer
	if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
		vertexBuffer.UnMap();
		vertexBuffer.Destroy();
		VK_CHECK_RESULT(device->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vertexBuffer, vertexBufferSize));
		vertexCount = imDrawData->TotalVtxCount;
		vertexBuffer.UnMap();
		vertexBuffer.Map();
		updateCmdBuffers = true;
	}

	// Index buffer
	if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
		indexBuffer.UnMap();
		indexBuffer.Destroy();
		VK_CHECK_RESULT(device->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &indexBuffer, indexBufferSize));
		indexCount = imDrawData->TotalIdxCount;
		indexBuffer.Map();
		updateCmdBuffers = true;
	}

	// Upload data
	ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
	ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	// Flush to make writes visible to GPU
	vertexBuffer.Flush();
	indexBuffer.Flush();

	return updateCmdBuffers;
}

void Render::Vulkan::UIOverlay::Draw(const VkCommandBuffer cmdBuf)
{
	ImDrawData* imDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

	pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	pushConstBlock.translate = glm::vec2(-1.0f);
	vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, &vertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuf, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

	for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[i];
		for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
			VkRect2D scissorRect;
			scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
			scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
			scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
			scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
			vkCmdSetScissor(cmdBuf, 0, 1, &scissorRect);
			vkCmdDrawIndexed(cmdBuf, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
			indexOffset += pcmd->ElemCount;
		}

		vertexOffset += cmd_list->VtxBuffer.Size;
	}
}

void Render::Vulkan::UIOverlay::Resize(uint32_t width, uint32_t height)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)(width), (float)(height));
}

void Render::Vulkan::UIOverlay::FreeResource()
{
	vertexBuffer.Destroy();
	indexBuffer.Destroy();
	vkDestroyImageView(device->logicalDevice, fontView, nullptr);
	vkDestroyImage(device->logicalDevice, fontImage, nullptr);
	vkFreeMemory(device->logicalDevice, fontMemory, nullptr);
	vkDestroySampler(device->logicalDevice, sampler, nullptr);
	vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(device->logicalDevice, descriptorPool, nullptr);
	vkDestroyPipelineLayout(device->logicalDevice, pipelineLayout, nullptr);
	vkDestroyPipeline(device->logicalDevice, pipeline, nullptr);
}

bool Render::Vulkan::UIOverlay::Header(const char* caption)
{
	return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
}

void Render::Vulkan::UIOverlay::Text(const char* pStr, ...)
{
	va_list args;
	va_start(args, pStr);
	ImGui::TextV(pStr, args);
	va_end(args);
}