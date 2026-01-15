#include "ApplicationWindow.h"

#define OBJECT_NUM 512
#define PI 3.141592653

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationMulThread ";
	m_camera.type = Camera::CameraType::lookat;
	m_camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	m_camera.setRotation(glm::vec3(0.0f));
	m_camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 1.0f, 256.0f);

	m_numThreads = std::thread::hardware_concurrency();

	printf("Thread Count : %d !!!!\n", m_numThreads);

	m_threadPool.SetThreadCount(m_numThreads);
	m_numObjectPerThread = OBJECT_NUM / m_numThreads;
	m_rndEngine.seed(time( 0 ));
}

ApplicationWin::~ApplicationWin()
{
	
}

float ApplicationWin::Rnd(float range)
{
	std::uniform_real_distribution<float> rndDist(0.0f, range);
	return rndDist(m_rndEngine);
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

void ApplicationWin::UpdateMatrices()
{
	m_matrix.projection = m_camera.matrices.perspective;
	m_matrix.view = m_camera.matrices.view;
}

void ApplicationWin::UpdateSecondaryCommandBuffers(VkCommandBufferInheritanceInfo inheritanceInfo)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();

	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

	VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
	VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);

	glm::mat4 mvp = m_matrix.projection * m_matrix.view;
	mvp[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mvp = glm::scale(mvp, glm::vec3(2.0f));
}

void ApplicationWin::PreparePipelines()
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Render::Vulkan::Initializer::PipelineLayoutCreateInfo(nullptr, 0);
	
	VkPushConstantRange pushConstantRange = Render::Vulkan::Initializer::PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(ThreadPushConstantBlock), 0);
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));


	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Render::Vulkan::
		Initializer::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = Render::Vulkan::
		Initializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
			VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = Render::Vulkan::
		Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = Render::Vulkan::
		Initializer::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = Render::Vulkan::
		Initializer::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = Render::Vulkan::
		Initializer::PipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = Render::Vulkan::
		Initializer::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = Render::Vulkan::
		Initializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);
	VkPipelineVertexInputStateCreateInfo vertexInputState = Render::Vulkan::
		Initializer::PipelineVertexInputStateCreateInfo();

	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttr0{};
	vertexInputAttr0.location = 0;
	vertexInputAttr0.binding = 0;
	vertexInputAttr0.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr0.offset = offsetof(Vertex, pos);

	VkVertexInputAttributeDescription vertexInputAttr1{};
	vertexInputAttr1.location = 1;
	vertexInputAttr1.binding = 0;
	vertexInputAttr1.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr1.offset = offsetof(Vertex, normal);

	VkVertexInputAttributeDescription vertexInputAttr2{};
	vertexInputAttr2.location = 2;
	vertexInputAttr2.binding = 0;
	vertexInputAttr2.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttr2.offset = offsetof(Vertex, color);

	std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
		vertexInputBinding
	};

	std::vector<VkVertexInputAttributeDescription> vertexInputAttrs = {
		vertexInputAttr0, vertexInputAttr1, vertexInputAttr2
	};

	vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
	vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttrs.size());
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttrs.data();

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	VkGraphicsPipelineCreateInfo pipelineCI = Render::Vulkan::Initializer::PipelineCreateInfo(m_pipelineLayout, m_renderPass, 0);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = &vertexInputState;

#if defined __ANDROID__
	shaderStages[0] = LoadShader("shaders/glsl/draw_multhread/draw_multhread.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/glsl/draw_multhread/draw_multhread.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
	shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_multhread/draw_multhread.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_multhread/draw_multhread.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &m_pipelines.phong));

	/*rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	depthStencilState.depthWriteEnable = VK_FALSE;
	shaderStages[0] = LoadShader("multithreading/starsphere.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("multithreading/starsphere.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &m_pipelines.startsphere));*/
}

void ApplicationWin::ThreadRenderCode(uint32_t threadIndex, uint32_t cmdBufferIndex, VkCommandBufferInheritanceInfo inheritanceInfo)
{
	ThreadData* thread = &m_threadData[threadIndex];
	ObjectData* objData = &thread->objectData[cmdBufferIndex];

	VkCommandBufferBeginInfo commandBufferBeginInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

	VkCommandBuffer cmdBuffer = thread->commandBuffer[m_currentBuffer][cmdBufferIndex];

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &commandBufferBeginInfo));

	VkViewport viewport = Render::Vulkan::Initializer::Viewport(width, height, 0, 1);
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines.phong);


	objData->rotation.y += 2.5f * objData->rotationSpeed * 1.0f;
	if (objData->rotation.y > 360.0f) {
		objData->rotation.y -= 360.0f;
	}
	objData->deltaT += 0.15f * 0.01f;
	if (objData->deltaT > 1.0f)
		objData->deltaT -= 1.0f;
	objData->pos.y = sin(glm::radians(objData->deltaT * 360.0f)) * 2.5f;

	objData->model = glm::translate(glm::mat4(1.0f), objData->pos);
	objData->model = glm::rotate(objData->model, -sinf(glm::radians(objData->deltaT * 360.0f)) * 0.25f, glm::vec3(objData->rotationDir, 0.0f, 0.0f));
	objData->model = glm::rotate(objData->model, glm::radians(objData->rotation.y), glm::vec3(0.0f, objData->rotationDir, 0.0f));
	objData->model = glm::rotate(objData->model, glm::radians(objData->deltaT * 360.0f), glm::vec3(0.0f, objData->rotationDir, 0.0f));
	objData->model = glm::scale(objData->model, glm::vec3(objData->scale));

	thread->pushConstBlock[cmdBufferIndex].mvp = m_matrix.projection * m_matrix.view * objData->model;

	vkCmdPushConstants(
		cmdBuffer,
		m_pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(ThreadPushConstantBlock),
		&thread->pushConstBlock[cmdBufferIndex]);

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_models.model.vertices.buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, m_models.model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmdBuffer, m_models.model.indices.count, 1, 0, 0, 0);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void ApplicationWin::PrepareUniformBuffer()
{
}

void ApplicationWin::BuildGraphicsCommandBuffer()
{
}

void ApplicationWin::PrepareMulthreadRenderer()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = Render::Vulkan::Initializer::CommandBufferAllocateInfo(m_cmdPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_secondaryCommandBuffer[i].background));
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &m_secondaryCommandBuffer[i].ui));
	}

	m_threadData.resize(m_numThreads);

	for (int i = 0; i < m_numThreads;i++)
	{
		ThreadData* curThreadData = &m_threadData[i];

		//初始化每个 线程的CommandBuffer
		VkCommandPoolCreateInfo cmdPoolInfo = Render::Vulkan::Initializer::CommandPoolCreateInfo();
		cmdPoolInfo.queueFamilyIndex = m_swapChain.queueNodeIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &curThreadData->commandPool));

		for (auto& commandBuffer : curThreadData->commandBuffer)
		{
			commandBuffer.resize(m_numObjectPerThread);

			VkCommandBufferAllocateInfo secondaryCmdBufAllocateInfo = Render::Vulkan::Initializer::CommandBufferAllocateInfo(curThreadData->commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY, static_cast<uint32_t>(commandBuffer.size()));
			VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &secondaryCmdBufAllocateInfo, commandBuffer.data()));
		}

		// 每个线程中object数据
		curThreadData->pushConstBlock.resize(m_numObjectPerThread);
		curThreadData->objectData.resize(m_numObjectPerThread);

		for (uint32_t j = 0; j < m_numObjectPerThread; j++) {
			float theta = 2.0f * float(PI) * Rnd(1.0f);
			float phi = acos(1.0f - 2.0f * Rnd(1.0f));
			curThreadData->objectData[j].pos = glm::vec3(sin(phi) * cos(theta), 0.0f, cos(phi)) * 35.0f;
			curThreadData->objectData[j].rotation = glm::vec3(0.0f, Rnd(360.0f), 0.0f);
			curThreadData->objectData[j].deltaT = Rnd(1.0f);
			curThreadData->objectData[j].rotationDir = (Rnd(100.0f) < 50.0f) ? 1.0f : -1.0f;
			curThreadData->objectData[j].rotationSpeed = (2.0f + Rnd(4.0f)) * curThreadData->objectData[j].rotationDir;
			curThreadData->objectData[j].scale = 0.75f + Rnd(0.5f);
			curThreadData->pushConstBlock[j].color = glm::vec3(Rnd(1.0f), Rnd(1.0f), Rnd(1.0f));
		}
	}
}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
#if defined __ANDROID__
	LoadAsset("mesh/ufo/retroufo_red_lowpoly.gltf"); // 加载图片
#else
	LoadAsset("./Asset/mesh/ufo/retroufo_red_lowpoly.gltf"); // 加载图片
#endif
	UpdateMatrices();
	PrepareMulthreadRenderer();
	PreparePipelines();
	prepared = true;
}

void ApplicationWin::UpdateCommandBuffers()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];

	std::vector<VkCommandBuffer> commandBuffers;
	
	VkCommandBufferBeginInfo cmdBufInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();
	
	VkClearValue clearValues[2]{};
	clearValues[0].color = { 0,0,0,1 };
	clearValues[1].depthStencil = { 1,0 };

	VkRenderPassBeginInfo renderPassBeginInfo = Render::Vulkan::Initializer::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentImageIndex];

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkCommandBufferInheritanceInfo inheritanceInfo = Render::Vulkan::Initializer::CommandBufferInheritanceInfo();
	inheritanceInfo.renderPass = renderPassBeginInfo.renderPass;
	inheritanceInfo.framebuffer = renderPassBeginInfo.framebuffer;

	for (uint32_t t = 0; t < m_numThreads; t++) {
		for (uint32_t i = 0; i < m_numObjectPerThread; i++) {
			m_threadPool.threads[t]->addJob([=, this] { ThreadRenderCode(t, i, inheritanceInfo); });
		}
	}

	m_threadPool.Wait();

	for (uint32_t t = 0; t < m_numThreads; t++) {
		for (uint32_t i = 0; i < m_numObjectPerThread; i++) {
			if (m_threadData[t].objectData[i].visible) {
				commandBuffers.push_back(m_threadData[t].commandBuffer[m_currentBuffer][i]);
			}
		}
	}

	vkCmdExecuteCommands(m_drawCmdBuffers[m_currentBuffer], static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkCmdEndRenderPass(m_drawCmdBuffers[m_currentBuffer]);

	VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[m_currentBuffer]));
}

void ApplicationWin::Render()
{
	if (!prepared)
		return;

	VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_waitFences[m_currentBuffer]));
	m_swapChain.AcquireNextImage(m_presentCompleteSemaphores[m_currentBuffer], m_currentImageIndex);

	UpdateMatrices();
	UpdateCommandBuffers();

	ApplicationBase::SubmitFrame(false);

	vkQueueWaitIdle(m_queue);

}

void ApplicationWin::LoadAsset(std::string fileNamePath)
{
	const uint32_t glTFLoadingFlags = VkModel::FileLoadingFlags::PreTransformVertices | VkModel::FileLoadingFlags::PreMultiplyVertexColors | VkModel::FileLoadingFlags::FlipY;
	m_models.model.LoadFromFile(fileNamePath, vulkanDevice, m_queue, glTFLoadingFlags);
}