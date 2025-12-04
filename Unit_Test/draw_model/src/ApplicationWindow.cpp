#include "ApplicationWindow.h"

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationDrawModel ";
	m_camera.type = Camera::CameraType::lookat;
	m_camera.flipY = true;
	m_camera.setPosition(glm::vec3(1.0f, 5.1f, -3.0f));
	m_camera.setRotation(glm::vec3(0.0f, 60.0f, 0.0f));
	m_camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 100.0f);
}

ApplicationWin::~ApplicationWin()
{
	
}

void ApplicationWin::DrawUI(const VkCommandBuffer cmdBuffer)
{
	const VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
	const VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	ui.Draw(cmdBuffer);
}

void ApplicationWin::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> descriptorTypeCounts;
	descriptorTypeCounts.resize(2);
	descriptorTypeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorTypeCounts[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	descriptorTypeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorTypeCounts[1].descriptorCount = m_glTFModel.images.size() * MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorTypeCounts.size());//descriptorTypeCounts.size()
	descriptorPoolInfo.pPoolSizes = descriptorTypeCounts.data();
	descriptorPoolInfo.maxSets = (m_glTFModel.images.size() + 1) * MAX_FRAMES_IN_FLIGHT;// type 个数 * MAX_FRAMES_IN_FLIGHT

	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));

}

VkShaderModule ApplicationWin::LoadSPIRVShader(const std::string& filename)
{
	size_t shaderSize;
	char* shaderCode{ nullptr };

#if defined (__ANDROID__)
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager,filename.c_str(), AASSET_MODE_STREAMING );
    assert(asset);
    shaderSize = AAsset_getLength(asset);
    assert(shaderSize > 0);

    shaderCode = new char[shaderSize];
    AAsset_read(asset, shaderCode, shaderSize);
    AAsset_close(asset);
#else
	std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (is.is_open())
	{
		shaderSize = is.tellg();
		is.seekg(0, std::ios::beg);
		// Copy file contents into a buffer
		shaderCode = new char[shaderSize];
		is.read(shaderCode, shaderSize);
		is.close();
		assert(shaderSize > 0);
	}
#endif
	if (shaderCode)
	{
		// Create a new shader module that will be used for pipeline creation
		VkShaderModuleCreateInfo shaderModuleCI{};
		shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCI.codeSize = shaderSize;
		shaderModuleCI.pCode = (uint32_t*)shaderCode;

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(m_device, &shaderModuleCI, nullptr, &shaderModule));

		delete[] shaderCode;

		return shaderModule;
	}
	else
	{
		std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
		return VK_NULL_HANDLE;
	}
}

void ApplicationWin::PreparePipeline()
{
	VkDescriptorSetLayoutBinding descrSetLayoutBinding0{};
	descrSetLayoutBinding0.binding = 0;
	descrSetLayoutBinding0.descriptorCount = 1;
	descrSetLayoutBinding0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descrSetLayoutBinding0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{
		descrSetLayoutBinding0
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout.matrices));

	VkDescriptorSetLayoutBinding descrSetLayoutBinding1{};
	descrSetLayoutBinding1.binding = 0;
	descrSetLayoutBinding1.descriptorCount = 1;
	descrSetLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descrSetLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings1{
		descrSetLayoutBinding1
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo1{};
	descriptorSetLayoutCreateInfo1.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo1.flags = 0;
	descriptorSetLayoutCreateInfo1.bindingCount = descriptorSetLayoutBindings1.size();
	descriptorSetLayoutCreateInfo1.pBindings = descriptorSetLayoutBindings1.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo1, nullptr, &m_descriptorSetLayout.textures));

	for (int i = 0;i < m_uniformDataBuffer.size();i++)
	{
		VkDescriptorSetAllocateInfo allocInfo0{};
		allocInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo0.descriptorPool = m_descriptorPool;
		allocInfo0.descriptorSetCount = 1;
		allocInfo0.pSetLayouts = &m_descriptorSetLayout.matrices;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo0, &m_descriptorSet[i]));

		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstSet = m_descriptorSet[i];
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &m_uniformDataBuffer[i].descriptor;

		vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);

		for (auto& image : m_glTFModel.images)
		{
			VkDescriptorSetAllocateInfo allocInfo0{};
			allocInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo0.descriptorPool = m_descriptorPool;
			allocInfo0.descriptorSetCount = 1;
			allocInfo0.pSetLayouts = &m_descriptorSetLayout.textures;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo0, &image.descriptorSet));
			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.dstSet = image.descriptorSet;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.pImageInfo = &image.texture.descirptor;
			vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);
		}
	}















	std::array<VkDescriptorSetLayout, 2> setLayouts = { m_descriptorSetLayout.matrices, m_descriptorSetLayout.textures };
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(glm::mat4);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.flags = 0;
	pipelineLayoutCI.setLayoutCount = setLayouts.size();
	pipelineLayoutCI.pSetLayouts = setLayouts.data();
	pipelineLayoutCI.pushConstantRangeCount = 1;
	pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout));

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.flags = 0;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = 0xf; // 0xf= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.flags = 0;

	VkPipelineMultisampleStateCreateInfo multiSampleState{};
	multiSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multiSampleState.flags = 0;

	std::vector<VkDynamicState> dynameicStateEnable{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynameicStateEnable.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynameicStateEnable.size());
	dynamicState.flags = 0;

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

#if defined (__ANDROID__)
	shaderStages[0] = LoadShader("shaders/glsl/draw_model/draw_model.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/glsl/draw_model/draw_model.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
	shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_model/draw_model.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_model/draw_model.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif


	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(GlTFModel::Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttr0{};
	vertexInputAttr0.location = 0;
	vertexInputAttr0.binding = 0;
	vertexInputAttr0.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr0.offset = offsetof(GlTFModel::Vertex, pos);

	VkVertexInputAttributeDescription vertexInputAttr1{};
	vertexInputAttr1.location = 1;
	vertexInputAttr1.binding = 0;
	vertexInputAttr1.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr1.offset = offsetof(GlTFModel::Vertex, normal);

	VkVertexInputAttributeDescription vertexInputAttr2{};
	vertexInputAttr2.location = 2;
	vertexInputAttr2.binding = 0;
	vertexInputAttr2.format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttr2.offset = offsetof(GlTFModel::Vertex, uv);

	VkVertexInputAttributeDescription vertexInputAttr3{};
	vertexInputAttr3.location = 3;
	vertexInputAttr3.binding = 0;
	vertexInputAttr3.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr3.offset = offsetof(GlTFModel::Vertex, color);

	std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
		vertexInputBinding
	};

	std::vector<VkVertexInputAttributeDescription> vertexInputAttrs = {
		vertexInputAttr0, vertexInputAttr1, vertexInputAttr2, vertexInputAttr3
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
	vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttrs.size());
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttrs.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = m_pipelineLayout;
	pipelineCreateInfo.renderPass = m_renderPass;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multiSampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.solid));

	rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizationState.lineWidth = 1.0f;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipelines.wireFrame));
}

void ApplicationWin::PrepareUniformBuffer()
{
	for (auto& item : m_uniformDataBuffer)
	{
		// 创建缓冲区（CreateBuffer）
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferCreateInfo.size = sizeof(UniformData);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//队列独占模式 只给一个人使用
		VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &item.buffer));

		//查询内存需求
		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(m_device, item.buffer, &memReqs);

		//分配内存
		VkMemoryAllocateInfo memAlloc = Render::Vulkan::Initializer::MemoryAllocInfo();
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &item.memory));

		item.alignment = memReqs.alignment;
		item.size = sizeof(UniformData);
		item.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		item.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		item.SetupDescriptor();

		//绑定内存将缓冲区和内存binding在一起
		VK_CHECK_RESULT(vkBindBufferMemory(m_device, item.buffer, item.memory, 0));

		//将CPU地址和GPU地址Map
		vkMapMemory(m_device, item.memory, 0, VK_WHOLE_SIZE, 0, &item.mapped);
	}
}

void ApplicationWin::BuildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];

	VkCommandBufferBeginInfo cmdBufBegInfo{};
	cmdBufBegInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufBegInfo));

	VkClearValue clearValue[2]{};
	clearValue[0].color = { 0.525f, 0.525f, 0.525f, 1.0f };
	clearValue[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentImageIndex];

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.01f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D rect2D{};
	rect2D.extent.width = width;
	rect2D.extent.height = height;
	rect2D.offset.x = 0;
	rect2D.offset.y = 0;
	vkCmdSetScissor(cmdBuffer, 0, 1, &rect2D);

	VkDeviceSize offsets[1] = { 0 };

	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet[m_currentBuffer], 0, nullptr);
	
	if (m_wireFrame)
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.wireFrame);
	else
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.solid);

	m_glTFModel.Draw(cmdBuffer, m_pipelineLayout);

	DrawUI(cmdBuffer);

	vkCmdEndRenderPass(cmdBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}


void ApplicationWin::GetEnabledFeatures(){
	// Fill mode non solid is required for wireframe display
	if (m_deviceFeatures.fillModeNonSolid) {
		m_enabledFeatures.fillModeNonSolid = VK_TRUE;
	};
}

void ApplicationWin::OnUpdateUIOverlay(Render::Vulkan::UIOverlay* overlay)
{
	if (overlay->Header("Settings")) {
		overlay->CheckBox("Wireframe", &m_wireFrame);
	}
}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	LoadAsset(); // 加载图片
	PrepareUniformBuffer();
	CreateDescriptorPool();
	PreparePipeline();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{
	m_uniform.projection = m_camera.matrices.perspective;
	m_uniform.model = m_camera.matrices.view;
	m_uniform.viewPos = m_camera.viewPos;
	memcpy(m_uniformDataBuffer[m_currentBuffer].mapped, &m_uniform, sizeof(UniformData));
}

void ApplicationWin::Render()
{
	if (!prepared)
		return;

	VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_waitFences[m_currentBuffer]));
	m_swapChain.AcquireNextImage(m_presentCompleteSemaphores[m_currentBuffer], m_currentImageIndex);

	UpdateUniformBuffers();
	BuildCommandBuffer();

	ApplicationBase::SubmitFrame(false);
	
	vkQueueWaitIdle(m_queue);
}

void ApplicationWin::LoadAsset()
{
#if defined(__ANDROID__)
    LoadGlTFFile("mesh/Sponza/Sponza.gltf", m_glTFModel);
#else
    LoadGlTFFile("./Asset/mesh/Sponza/Sponza.gltf", m_glTFModel);
#endif

}