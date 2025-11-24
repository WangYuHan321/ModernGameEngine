#include "ApplicationWindow.h"

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationComputeShader ";
	m_camera.type = Camera::CameraType::lookat;
	m_camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	m_camera.setRotation(glm::vec3(0.0f));
	m_camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 1.0f, 256.0f);
}

ApplicationWin::~ApplicationWin()
{
	
}

void ApplicationWin::CreateQuad()
{
	std::vector<Vertex> vertices = {
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } }
	};

	uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);
	std::vector<uint32_t> indexBuffer{ 0, 1, 2, 2,3,0 };
	m_indexCount = static_cast<uint32_t>(indexBuffer.size());
	uint32_t indexBufferSize = m_indexCount * sizeof(uint32_t);

	VkMemoryAllocateInfo memAlloc = Render::Vulkan::Initializer::MemoryAllocInfo();
	VkMemoryRequirements memReqs;

	struct StagingBuffer {
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	struct {
		StagingBuffer vertices;
		StagingBuffer indices;
	} stagingBuffers{};

	void* data;

	// Vertex buffer
	VkBufferCreateInfo vertexBufferInfoCI{};
	vertexBufferInfoCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfoCI.size = vertexBufferSize;
	// Buffer is used as the copy source
	vertexBufferInfoCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Create a host-visible buffer to copy the vertex data to (staging buffer)
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &vertexBufferInfoCI, nullptr, &stagingBuffers.vertices.buffer));
	vkGetBufferMemoryRequirements(m_device, stagingBuffers.vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	// Request a host visible memory type that can be used to copy our data to
	// Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
	// Map and copy
	VK_CHECK_RESULT(vkMapMemory(m_device, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
	memcpy(data, vertices.data(), vertexBufferSize);
	vkUnmapMemory(m_device, stagingBuffers.vertices.memory);
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	vertexBufferInfoCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &vertexBufferInfoCI, nullptr, &m_vertexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device, m_vertexBuffer.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &m_vertexBuffer.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, m_vertexBuffer.buffer, m_vertexBuffer.memory, 0));

	// Index buffer
	VkBufferCreateInfo indexbufferCI{};
	indexbufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexbufferCI.size = indexBufferSize;
	indexbufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Copy index data to a buffer visible to the host (staging buffer)
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &indexbufferCI, nullptr, &stagingBuffers.indices.buffer));
	vkGetBufferMemoryRequirements(m_device, stagingBuffers.indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &stagingBuffers.indices.memory));
	VK_CHECK_RESULT(vkMapMemory(m_device, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
	memcpy(data, indexBuffer.data(), indexBufferSize);
	vkUnmapMemory(m_device, stagingBuffers.indices.memory);
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

	// Create destination buffer with device only visibility
	indexbufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &indexbufferCI, nullptr, &m_indexBuffer.buffer));
	vkGetBufferMemoryRequirements(m_device, m_indexBuffer.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &m_indexBuffer.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, m_indexBuffer.buffer, m_indexBuffer.memory, 0));

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
	VkCommandBuffer copyCmd;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = m_cmdPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &copyCmd));

	VkCommandBufferBeginInfo cmdBufInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion{};
	// Vertex buffer
	copyRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, m_vertexBuffer.buffer, 1, &copyRegion);
	// Index buffer
	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, m_indexBuffer.buffer, 1, &copyRegion);
	VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

	// Submit the command buffer to the queue to finish the copy
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.flags = 0;
	VkFence fence;
	VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCI, nullptr, &fence));

	// Submit to the queue
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, fence));
	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(m_device, fence, nullptr);
	vkFreeCommandBuffers(m_device, m_cmdPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(m_device, stagingBuffers.vertices.buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffers.vertices.memory, nullptr);
	vkDestroyBuffer(m_device, stagingBuffers.indices.buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffers.indices.memory, nullptr);

}

void ApplicationWin::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> descriptorTypeCounts;
	descriptorTypeCounts.resize(3);
	descriptorTypeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorTypeCounts[0].descriptorCount = MAX_FRAMES_IN_FLIGHT * 2;// preCompute postCompute  （2个） 1个unfiromBuffer

	descriptorTypeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorTypeCounts[1].descriptorCount = MAX_FRAMES_IN_FLIGHT * 2;// preCompute postCompute  （2个） 1个unfiromBuffer

	descriptorTypeCounts[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptorTypeCounts[2].descriptorCount = MAX_FRAMES_IN_FLIGHT * 2;// 这里 * 2 对应 ComputerShader 2个Image

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(3);//descriptorTypeCounts.size()
	descriptorPoolInfo.pPoolSizes = descriptorTypeCounts.data();
	descriptorPoolInfo.maxSets = 3 * MAX_FRAMES_IN_FLIGHT;// type 个数 * MAX_FRAMES_IN_FLIGHT

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

void ApplicationWin::PrepareGraphicsPipeline()
{
	//设置 Descriptor Layout
	VkDescriptorType vkDescriptType0 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	VkDescriptorType vkDescriptType1 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;//VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE

	VkDescriptorSetLayoutBinding setLayoutBinding0{};
	setLayoutBinding0.binding = 0;
	setLayoutBinding0.descriptorType = vkDescriptType0;
	setLayoutBinding0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	setLayoutBinding0.descriptorCount = 1;

	VkDescriptorSetLayoutBinding setLayoutBinding1{};
	setLayoutBinding1.binding = 1;
	setLayoutBinding1.descriptorType = vkDescriptType1;
	setLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBinding1.descriptorCount = 1;

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBinding = {
				setLayoutBinding0,setLayoutBinding1
	};

	VkDescriptorSetLayoutCreateInfo descriptorCreateInfo{};
	descriptorCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorCreateInfo.bindingCount = setLayoutBinding.size();
	descriptorCreateInfo.pBindings = setLayoutBinding.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorCreateInfo, nullptr, &m_graphics.descriptorSetLayout));
	for (auto i = 0; i < m_graphics.uniformBuffers.size(); i++)
	{
		VkDescriptorSetAllocateInfo descriptorAllocInfo0 {};
		descriptorAllocInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorAllocInfo0.descriptorPool = m_descriptorPool;
		descriptorAllocInfo0.descriptorSetCount = 1;// 这里 只有1个 preCompute
		descriptorAllocInfo0.pSetLayouts = &m_graphics.descriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &descriptorAllocInfo0, &m_graphics.descriptorSets[i].preCompute));
		VkWriteDescriptorSet writeDescriptorSet0{};
		writeDescriptorSet0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet0.dstSet = m_graphics.descriptorSets[i].preCompute;
		writeDescriptorSet0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet0.dstBinding = 0;
		writeDescriptorSet0.pBufferInfo = &m_graphics.uniformBuffers[i].descriptor;
		writeDescriptorSet0.descriptorCount = 1;//更新一个Buffer

		VkWriteDescriptorSet writeDescriptorSet1{};
		writeDescriptorSet1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet1.dstSet = m_graphics.descriptorSets[i].preCompute;
		writeDescriptorSet1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet1.dstBinding = 1;
		writeDescriptorSet1.pImageInfo = &m_textureColorMap.descirptor;
		writeDescriptorSet1.descriptorCount = 1;//更新一个Buffer
		
		std::vector<VkWriteDescriptorSet> writeDescriptorSets0 = { writeDescriptorSet0 , writeDescriptorSet1 };

		vkUpdateDescriptorSets(m_device, writeDescriptorSets0.size(), writeDescriptorSets0.data(), 0, nullptr);

		VkDescriptorSetAllocateInfo descriptorAllocInfo1 {};
		descriptorAllocInfo1.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorAllocInfo1.descriptorPool = m_descriptorPool;
		descriptorAllocInfo1.descriptorSetCount = 1;// 这里 只有1个 preCompute
		descriptorAllocInfo1.pSetLayouts = &m_graphics.descriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &descriptorAllocInfo1, &m_graphics.descriptorSets[i].postCompute));
		VkWriteDescriptorSet writeDescriptorSet00{};
		writeDescriptorSet00.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet00.dstSet = m_graphics.descriptorSets[i].postCompute;
		writeDescriptorSet00.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet00.dstBinding = 0;
		writeDescriptorSet00.pBufferInfo = &m_graphics.uniformBuffers[i].descriptor;
		writeDescriptorSet00.descriptorCount = 1;//更新一个Buffer

		VkWriteDescriptorSet writeDescriptorSet11{};
		writeDescriptorSet11.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet11.dstSet = m_graphics.descriptorSets[i].postCompute;
		writeDescriptorSet11.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet11.dstBinding = 1;
		writeDescriptorSet11.pImageInfo = &m_storageImage.descirptor;
		writeDescriptorSet11.descriptorCount = 1;//更新一个Buffer

		std::vector<VkWriteDescriptorSet> writeDescriptorSets1 = { writeDescriptorSet00 , writeDescriptorSet11 };

		vkUpdateDescriptorSets(m_device, writeDescriptorSets1.size(), writeDescriptorSets1.data(), 0, nullptr);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_graphics.descriptorSetLayout;

	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_graphics.pipelineLayout));

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

	shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_compute/texture.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_compute/texture.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttr0{};
	vertexInputAttr0.location = 0;
	vertexInputAttr0.binding = 0;
	vertexInputAttr0.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr0.offset = offsetof(Vertex, position);

	VkVertexInputAttributeDescription vertexInputAttr1{};
	vertexInputAttr1.location = 1;
	vertexInputAttr1.binding = 0;
	vertexInputAttr1.format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttr1.offset = offsetof(Vertex, uv);

	std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
		vertexInputBinding
	};

	std::vector<VkVertexInputAttributeDescription> vertexInputAttrs = {
		vertexInputAttr0, vertexInputAttr1
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
	vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttrs.size());
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttrs.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = m_graphics.pipelineLayout;
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

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &m_graphics.pipeline));
}

void ApplicationWin::PrepareComputePipeline()
{
	vkGetDeviceQueue(m_device, vulkanDevice->queueFamilyIndices.compute, 0, &m_compute.queue);

	VkCommandPoolCreateInfo cmdPoolInfo{};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkanDevice->queueFamilyIndices.compute;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_compute.commandPool));

	VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = m_compute.commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	for (auto& commandBuffer : m_compute.commandBuffers)
	{
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &commandBuffer));
	}

	for (auto& fence : m_compute.fences)
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence));
	}

	VkDescriptorType vkDescriptType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

	VkDescriptorSetLayoutBinding setLayoutBinding0{};
	setLayoutBinding0.binding = 0;
	setLayoutBinding0.descriptorType = vkDescriptType;
	setLayoutBinding0.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	setLayoutBinding0.descriptorCount = 1;

	VkDescriptorSetLayoutBinding setLayoutBinding1{};
	setLayoutBinding1.binding = 1;
	setLayoutBinding1.descriptorType = vkDescriptType;
	setLayoutBinding1.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	setLayoutBinding1.descriptorCount = 1;

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		setLayoutBinding0, setLayoutBinding1
	};

	VkDescriptorSetLayoutCreateInfo descriptorCreateInfo{};
	descriptorCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorCreateInfo.pBindings = setLayoutBindings.data();
	descriptorCreateInfo.bindingCount = setLayoutBindings.size();
	descriptorCreateInfo.flags = 0;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorCreateInfo, nullptr, &m_compute.descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Render::Vulkan::Initializer::PipelineLayoutCreateInfo(&m_compute.descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_compute.pipelineLayout));

	VkDescriptorSetAllocateInfo setAllocateInfo{};
	setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocateInfo.descriptorPool = m_descriptorPool;
	setAllocateInfo.pSetLayouts = &m_compute.descriptorSetLayout;
	setAllocateInfo.descriptorSetCount = 1;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &setAllocateInfo, &m_compute.descriptorSet));
	
	VkWriteDescriptorSet writeDescriptorSet0{};
	writeDescriptorSet0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet0.dstSet = m_compute.descriptorSet;
	writeDescriptorSet0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeDescriptorSet0.dstBinding = 0;
	writeDescriptorSet0.pImageInfo = &m_textureColorMap.descirptor;
	writeDescriptorSet0.descriptorCount = 1;//更新一个Buffer

	VkWriteDescriptorSet writeDescriptorSet1{};
	writeDescriptorSet1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet1.dstSet = m_compute.descriptorSet;
	writeDescriptorSet1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeDescriptorSet1.dstBinding = 1;
	writeDescriptorSet1.pImageInfo = &m_storageImage.descirptor;
	writeDescriptorSet1.descriptorCount = 1;//更新一个Buffer
	
	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			writeDescriptorSet0, writeDescriptorSet1
	};

	vkUpdateDescriptorSets(m_device, 2, writeDescriptorSets.data(), 0, nullptr);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = m_compute.pipelineLayout;
	computePipelineCreateInfo.flags = 0;

	for (auto item : m_filterName)
	{
		std::string strPath = "./Asset/shader/glsl/draw_compute/" + item + ".comp.spv";
		computePipelineCreateInfo.stage = LoadShader(strPath.c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
		
		VkPipeline pipeline;
		VK_CHECK_RESULT(vkCreateComputePipelines(m_device, m_pipelineCache, 1, &computePipelineCreateInfo, nullptr, &pipeline));
		m_compute.pipelines.push_back(pipeline);
	}
}


void ApplicationWin::PrepareUniformBuffer()
{
	for (auto& item : m_graphics.uniformBuffers)
	{
		// 创建缓冲区（CreateBuffer）
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferCreateInfo.size = sizeof(Graphics::UniformData);
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
		item.size = sizeof(Graphics::UniformData);
		item.usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		item.memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		item.SetupDescriptor();

		//绑定内存将缓冲区和内存binding在一起
		VK_CHECK_RESULT(vkBindBufferMemory(m_device, item.buffer, item.memory, 0));

		//将CPU地址和GPU地址Map
		vkMapMemory(m_device, item.memory, 0, VK_WHOLE_SIZE, 0, &item.mapped);
	}
}

void ApplicationWin::BuildGraphicsCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];

	VkCommandBufferBeginInfo cmdBufBegInfo{};
	cmdBufBegInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufBegInfo));

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	// We won't be changing the layout of the image
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.image = m_storageImage.image;
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_FLAGS_NONE,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

	VkClearValue clearValue[2]{};
	clearValue[0].color = { 0.025f, 0.025f, 0.025f, 1.0f };
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
	viewport.width = width * 0.5f;
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
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_vertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	//先绘制左边
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics.pipelineLayout, 0, 1, &m_graphics.descriptorSets[m_currentBuffer].preCompute, 0, nullptr);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics.pipeline);

	vkCmdDrawIndexed(cmdBuffer, m_indexCount, 1, 0, 0, 0);

	//绘制右边
	viewport.x = (float)width / 2.0f;
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics.pipelineLayout, 0, 1, &m_graphics.descriptorSets[m_currentBuffer].postCompute, 0, nullptr);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics.pipeline);

	vkCmdDrawIndexed(cmdBuffer, m_indexCount, 1, 0, 0, 0);

	vkCmdEndRenderPass(cmdBuffer);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void ApplicationWin::BuildComputeCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_compute.commandBuffers[m_currentBuffer];
	VkCommandBufferBeginInfo cmdBufBegInfo{};
	cmdBufBegInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufBegInfo));

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_compute.pipelines[m_compute.pipelineIndex]);
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_compute.pipelineLayout, 0, 1, &m_compute.descriptorSet, 0, 0);
	vkCmdDispatch(cmdBuffer, m_storageImage.width / 16, m_storageImage.height / 16, 1);
	vkEndCommandBuffer(cmdBuffer);
}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	CreateQuad();// 加载模型
	LoadAsset(); // 加载图片
	PrepareUniformBuffer();
	CreateDescriptorPool();
	PrepareGraphicsPipeline();
	PrepareComputePipeline();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{
	m_camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 1.0f, 256.0f);
	m_graphics.uniformData.projection = m_camera.matrices.perspective;
	m_graphics.uniformData.modelView = m_camera.matrices.view;
	memcpy(m_graphics.uniformBuffers[m_currentBuffer].mapped, &m_graphics.uniformData, sizeof(Graphics::UniformData));
}

void ApplicationWin::Render()
{
	if (!prepared)
		return;

	//Vulkan CONCURRENT 模式只是解决了资源的"访问权限"问题，完全不处理"执行同步"！
	//所以这里必须 等待计算队列完成
	vkWaitForFences(m_device, 1, &m_compute.fences[m_currentBuffer], VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &m_compute.fences[m_currentBuffer]);
	BuildComputeCommandBuffer();

	VkSubmitInfo computeSubmitInfo{};
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &m_compute.commandBuffers[m_currentBuffer];
	VK_CHECK_RESULT(vkQueueSubmit(m_compute.queue, 1, &computeSubmitInfo, m_compute.fences[m_currentBuffer]));


	VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_waitFences[m_currentBuffer]));
	m_swapChain.AcquireNextImage(m_presentCompleteSemaphores[m_currentBuffer], m_currentImageIndex);

	UpdateUniformBuffers();
	BuildGraphicsCommandBuffer();

	ApplicationBase::SubmitFrame(false);
	
}

void ApplicationWin::LoadAsset()
{
	const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	m_textureColorMap.LoadFromFile("./Asset/texture/lena.jpg", format, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &formatProperties);
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

	m_storageImage.width = m_textureColorMap.width;
	m_storageImage.height = m_textureColorMap.height;

	VkImageCreateInfo imageCreateInfo = Render::Vulkan::Initializer::ImageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = { m_storageImage.width, m_storageImage.height, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageCreateInfo.flags = 0;


	std::vector<uint32_t> queueFamilyIndices;
	if (vulkanDevice->queueFamilyIndices.graphics != vulkanDevice->queueFamilyIndices.compute) {
		queueFamilyIndices = {
			vulkanDevice->queueFamilyIndices.graphics,
			vulkanDevice->queueFamilyIndices.compute
		};
		imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		imageCreateInfo.queueFamilyIndexCount = 2;
		imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	VK_CHECK_RESULT(vkCreateImage(m_device, &imageCreateInfo, nullptr, &m_storageImage.image));

	VkMemoryAllocateInfo memAllocInfo = Render::Vulkan::Initializer::MemoryAllocInfo();
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(m_device, m_storageImage.image, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAllocInfo, nullptr, &m_storageImage.deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(m_device, m_storageImage.image, m_storageImage.deviceMemory, 0));

	// Transition image to the general layout, so we can use it as a storage image in the compute shader
	VkCommandBuffer layoutCmd = vulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	m_storageImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	Render::Vulkan::Tool::SetImageLayout(layoutCmd, m_storageImage.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, m_storageImage.imageLayout);
	vulkanDevice->FlushCommandBuffer(layoutCmd, m_queue, true);

	// Create sampler
	VkSamplerCreateInfo sampler = Render::Vulkan::Initializer::SamplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(m_device, &sampler, nullptr, &m_storageImage.sampler));

	// Create image view
	VkImageViewCreateInfo view = Render::Vulkan::Initializer::ImageViewCreateInfo();
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	view.image = m_storageImage.image;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &view, nullptr, &m_storageImage.imageView));

	// Initialize a descriptor for later use
	m_storageImage.descirptor.imageLayout = m_storageImage.imageLayout;
	m_storageImage.descirptor.imageView = m_storageImage.imageView;
	m_storageImage.descirptor.sampler = m_storageImage.sampler;
	m_storageImage.device = vulkanDevice;
}