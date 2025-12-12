#include "ApplicationWindow.h"
#include <random>

// plants.gltf 模型文件可能包含多个mesh
// 每个mesh对应一种植物类型，与纹理数组层对应

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationDrawIndirectDraw ";
	m_camera.type = Camera::CameraType::firstperson;
	m_camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 512.0f);
	m_camera.setRotation(glm::vec3(-12.0f, 159.0f, 0.0f));
	m_camera.setTranslation(glm::vec3(0.4f, 1.25f, 0.0f));
	m_camera.movementSpeed = 5.0f;
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

void ApplicationWin::SetupDescriptors()
{
	std::vector<VkDescriptorPoolSize> poolSize = {
		Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT),
		Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * MAX_FRAMES_IN_FLIGHT)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = Render::Vulkan::Initializer::
		DescriptorPoolCreateInfo(poolSize, MAX_FRAMES_IN_FLIGHT);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2)
	};
	VkDescriptorSetLayoutCreateInfo descriptorLayout = Render::Vulkan::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayout, nullptr, &m_descriptorSetLayout));

	//set
	VkDescriptorSetAllocateInfo allocInfo = Render::Vulkan::Initializer::DescriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet));

	std::vector<VkWriteDescriptorSet> writerDescriptorSets = {
		Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &m_uniformDataBuffer.descriptor),
		Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,&m_texture.plants.descirptor),
		Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2,&m_texture.ground.descirptor)
	};

	vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writerDescriptorSets.size()), writerDescriptorSets.data(), 0, nullptr);
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
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Render::Vulkan::Initializer::PipelineLayoutCreateInfo(&m_descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Render::Vulkan::Initializer::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = Render::Vulkan::Initializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = Render::Vulkan::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = Render::Vulkan::Initializer::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = Render::Vulkan::Initializer::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = Render::Vulkan::Initializer::PipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = Render::Vulkan::Initializer::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = Render::Vulkan::Initializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = Render::Vulkan::Initializer::PipelineCreateInfo(m_pipelineLayout, m_renderPass);
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	//这里是2个input state 1个是instanced 1个是非instanced
	VkPipelineVertexInputStateCreateInfo inputState = Render::Vulkan::Initializer::PipelineVertexInputStateCreateInfo();
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	bindingDescriptions = {
		Render::Vulkan::Initializer::VertexInputBindingDescription(0, sizeof(VkModel::Vertex), VK_VERTEX_INPUT_RATE_VERTEX),
		Render::Vulkan::Initializer::VertexInputBindingDescription(1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
	};

	attributeDescriptions = {
		Render::Vulkan::Initializer::VertexInputAttributeDescription(0,0, VK_FORMAT_R32G32B32_SFLOAT,0),
		Render::Vulkan::Initializer::VertexInputAttributeDescription(0,1, VK_FORMAT_R32G32B32_SFLOAT,sizeof(float) * 3),
		Render::Vulkan::Initializer::VertexInputAttributeDescription(0,2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),
		Render::Vulkan::Initializer::VertexInputAttributeDescription(0,3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),

		Render::Vulkan::Initializer::VertexInputAttributeDescription(1,4, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, pos)),
		Render::Vulkan::Initializer::VertexInputAttributeDescription(1,5, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, rot)),
		Render::Vulkan::Initializer::VertexInputAttributeDescription(1,6, VK_FORMAT_R32_SFLOAT, offsetof(InstanceData, scale)),
		Render::Vulkan::Initializer::VertexInputAttributeDescription(1,7, VK_FORMAT_R32_SINT, offsetof(InstanceData, texIndex)),
	};
	inputState.pVertexBindingDescriptions = bindingDescriptions.data();
	inputState.pVertexAttributeDescriptions = attributeDescriptions.data();
	inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

	pipelineCreateInfo.pVertexInputState = &inputState;
	//VkPipelineRasterizationStateCreateInfo rasterizationStateNew = Render::Vulkan::Initializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_LINE, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	//pipelineCreateInfo.pRasterizationState = &rasterizationStateNew;

#if defined (__ANDROID__)
    shaderStages[0] = LoadShader("shaders/glsl/draw_indirectDraw/indirectDraw.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = LoadShader("shaders/glsl/draw_indirectDraw/indirectDraw.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
	shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_indirectDraw/indirectDraw.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_indirectDraw/indirectDraw.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline.plants));
	//pipelineCreateInfo.pRasterizationState = &rasterizationState;

	inputState.vertexBindingDescriptionCount = 1;
	inputState.vertexAttributeDescriptionCount = 4;
#if defined (__ANDROID__)
    shaderStages[0] = LoadShader("shaders/glsl/draw_indirectDraw/ground.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = LoadShader("shaders/glsl/draw_indirectDraw/ground.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
	shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_indirectDraw/ground.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_indirectDraw/ground.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline.ground));

#if defined (__ANDROID__)
    shaderStages[0] = LoadShader("shaders/glsl/draw_indirectDraw/skysphere.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = LoadShader("shaders/glsl/draw_indirectDraw/skysphere.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
    shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_indirectDraw/skysphere.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_indirectDraw/skysphere.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif
	depthStencilState.depthWriteEnable = VK_FALSE;
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_pipeline.skySphere));
}

void ApplicationWin::PrepareUniformBuffer()
{
	// 创建缓冲区（CreateBuffer）
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCreateInfo.size = sizeof(UniformData);
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;//队列独占模式 只给一个人使用
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_uniformDataBuffer.buffer));

	//查询内存需求
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_device, m_uniformDataBuffer.buffer, &memReqs);

	//分配内存
	VkMemoryAllocateInfo memAlloc = Render::Vulkan::Initializer::MemoryAllocInfo();
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &m_uniformDataBuffer.memory));

	m_uniformDataBuffer.alignment = memReqs.alignment;
	m_uniformDataBuffer.size = sizeof(UniformData);
	m_uniformDataBuffer.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	m_uniformDataBuffer.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	m_uniformDataBuffer.SetupDescriptor();

	//绑定内存将缓冲区和内存binding在一起
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, m_uniformDataBuffer.buffer, m_uniformDataBuffer.memory, 0));

	//将CPU地址和GPU地址Map
	vkMapMemory(m_device, m_uniformDataBuffer.memory, 0, VK_WHOLE_SIZE, 0, &m_uniformDataBuffer.mapped);
}

void ApplicationWin::BuildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];

	VkCommandBufferBeginInfo cmdBufInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();

	VkClearValue clearValue[2];
	clearValue[0].color = { { 0.18f, 0.27f, 0.5f, 0.0f } };
	clearValue[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = Render::Vulkan::Initializer::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;



	// Set target frame buffer
	renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentImageIndex];

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, NULL);

	// Skysphere
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.skySphere);
	m_model.skySphere.Draw(cmdBuffer);
	// Ground
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.ground);
	m_model.ground.Draw(cmdBuffer);

	// [POI] Instanced multi draw rendering of the plants
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.plants);
	// Binding point 0 : Mesh vertex buffer
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_model.plants.vertices.buffer, offsets);
	// Binding point 1 : Instance data buffer
	vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &m_instanceBuffer.buffer, offsets);

	vkCmdBindIndexBuffer(cmdBuffer, m_model.plants.indices.buffer, 0, VK_INDEX_TYPE_UINT32);

	// If the multi draw feature is supported:
	// One draw call for an arbitrary number of objects
	// Index offsets and instance count are taken from the indirect buffer
	if (vulkanDevice->features.multiDrawIndirect)
	{
		vkCmdDrawIndexedIndirect(cmdBuffer, m_indirectCommandBuffer.buffer, 0, m_indirectDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}
	else
	{
		// If multi draw is not available, we must issue separate draw commands
		for (auto j = 0; j < m_indirectCommands.size(); j++)
		{
			vkCmdDrawIndexedIndirect(cmdBuffer, m_indirectCommandBuffer.buffer, j * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
		}
	}

	DrawUI(cmdBuffer);

	vkCmdEndRenderPass(cmdBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void ApplicationWin::PrepareInstanceData()
{
	std::vector<InstanceData> instanceData;
	instanceData.resize(m_objectCount);

	std::default_random_engine rndEngine(time(0));
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);

	for (uint32_t i = 0; i < m_objectCount; i++) 
	{
		float theta = 2 * float(3.14) * uniformDist(rndEngine);
		float phi = acos(1 - 2 * uniformDist(rndEngine));
		instanceData[i].rot = glm::vec3(0.0f, float(3.14) * uniformDist(rndEngine), 0.0f);
		instanceData[i].pos = glm::vec3(sin(phi) * cos(theta), 0.0f, cos(phi)) * 20.0f;
		instanceData[i].scale = 1.0f + uniformDist(rndEngine) * 2.0f;
		instanceData[i].texIndex = i % OBJECT_INSTANCE_COUNT;
	}

	Render::Vulkan::Buffer stagingBuffer;
	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		instanceData.size() * sizeof(InstanceData),
		instanceData.data()));

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_instanceBuffer,
		stagingBuffer.size));

	vulkanDevice->CopyBuffer(&stagingBuffer, &m_instanceBuffer, m_queue);
	stagingBuffer.Destroy();
}

void ApplicationWin::PrepareIndirectCommandBuffer()
{
	m_indirectCommands.clear();

	uint32_t m = 0;

	for (auto& node : m_model.plants.nodes)
	{
		if (node->mesh->primitives.size() > 0)
		{
			VkDrawIndexedIndirectCommand indirectCmd{};
			indirectCmd.instanceCount = OBJECT_INSTANCE_COUNT;
			indirectCmd.firstIndex = m * OBJECT_INSTANCE_COUNT;
			indirectCmd.firstIndex = node->mesh->primitives[0]->firstIndex;
			indirectCmd.indexCount = node->mesh->primitives[0]->indexCount;

			m_indirectCommands.push_back(indirectCmd);
			m++;
		}

	}

	m_indirectDrawCount = static_cast<uint32_t>(m_indirectCommands.size());

	m_objectCount = 0;
	for (auto indirectCmd : m_indirectCommands)
	{
		m_objectCount += indirectCmd.instanceCount;
	}

	Render::Vulkan::Buffer stagingBuffer;

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		m_indirectCommands.size() * sizeof(VkDrawIndexedIndirectCommand),
		m_indirectCommands.data()));

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&m_indirectCommandBuffer,
		stagingBuffer.size));

	vulkanDevice->CopyBuffer(&stagingBuffer, &m_indirectCommandBuffer, m_queue);
	stagingBuffer.Destroy();
}


void ApplicationWin::GetEnabledFeatures(){
	// Example uses multi draw indirect if available
	if (m_deviceFeatures.multiDrawIndirect) {
		m_enabledFeatures.multiDrawIndirect = VK_TRUE;
	}
	// Enable anisotropic filtering if supported
	if (m_deviceFeatures.samplerAnisotropy) {
		m_enabledFeatures.samplerAnisotropy = VK_TRUE;
	}
}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	LoadAsset(); // 加载图片
	PrepareIndirectCommandBuffer();
	PrepareInstanceData();
	PrepareUniformBuffer();
	SetupDescriptors();
	PreparePipeline();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{
	m_uniformData.projection = m_camera.matrices.perspective;
	m_uniformData.view = m_camera.matrices.view;
	memcpy(m_uniformDataBuffer.mapped, &m_uniformData, sizeof(UniformData));
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
	const uint32_t glTFLoadingFlags = Render::Vulkan::VkModel::FileLoadingFlags::PreTransformVertices
		| Render::Vulkan::VkModel::FileLoadingFlags::PreMultiplyVertexColors |
		Render::Vulkan::VkModel::FileLoadingFlags::FlipY;

#if defined (__ANDROID__)
    m_model.plants.LoadFromFile("mesh/IndirectDraw/plants.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
	m_model.ground.LoadFromFile("mesh/IndirectDraw/plane_circle.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
	m_model.skySphere.LoadFromFile("mesh/IndirectDraw/sphere.gltf", vulkanDevice, m_queue, glTFLoadingFlags);

	std::vector<std::string> strFileVec = {
		"mesh/IndirectDraw/0.png",
		"mesh/IndirectDraw/1.png",
		"mesh/IndirectDraw/2.png",
		"mesh/IndirectDraw/3.png",
		"mesh/IndirectDraw/4.png",
		"mesh/IndirectDraw/5.png",
		"mesh/IndirectDraw/6.png",
		"mesh/IndirectDraw/7.png",
		"mesh/IndirectDraw/8.png",
		"mesh/IndirectDraw/9.png",
		"mesh/IndirectDraw/10.png",
		"mesh/IndirectDraw/11.png"
	};

	m_texture.plants.mipLevels = 10;
	m_texture.ground.mipLevels = 10;
	m_texture.plants.LoadFromFile(strFileVec, VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue);
	m_texture.ground.LoadFromFile("mesh/IndirectDraw/test.png", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue);
#else
	m_model.plants.LoadFromFile("./Asset/mesh/IndirectDraw/plants.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
	m_model.ground.LoadFromFile("./Asset/mesh/IndirectDraw/plane_circle.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
	m_model.skySphere.LoadFromFile("./Asset/mesh/IndirectDraw/sphere.gltf", vulkanDevice, m_queue, glTFLoadingFlags);

	std::vector<std::string> strFileVec = {
		"./Asset/mesh/IndirectDraw/0.png",
		"./Asset/mesh/IndirectDraw/1.png",
		"./Asset/mesh/IndirectDraw/2.png",
		"./Asset/mesh/IndirectDraw/3.png",
		"./Asset/mesh/IndirectDraw/4.png",
		"./Asset/mesh/IndirectDraw/5.png",
		"./Asset/mesh/IndirectDraw/6.png",
		"./Asset/mesh/IndirectDraw/7.png",
		"./Asset/mesh/IndirectDraw/8.png",
		"./Asset/mesh/IndirectDraw/9.png",
		"./Asset/mesh/IndirectDraw/10.png",
		"./Asset/mesh/IndirectDraw/11.png"
	};

	m_texture.plants.mipLevels = 10;
	m_texture.ground.mipLevels = 10;
	m_texture.plants.LoadFromFile(strFileVec, VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue);
	m_texture.ground.LoadFromFile("./Asset/mesh/IndirectDraw/test.png", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue);
#endif
}
