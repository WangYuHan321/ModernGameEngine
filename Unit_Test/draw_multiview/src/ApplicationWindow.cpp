#include "ApplicationWindow.h"

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationMultiviewView ";
	m_camera.type = Camera::CameraType::firstperson;
	m_camera.setRotation(glm::vec3(0.0f, 90.0f, 0.0f));
	m_camera.setTranslation(glm::vec3(7.0f, 3.2f, 0.0f));
	m_camera.movementSpeed = 5.0f;

	m_enabledDeviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
	m_enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	m_physicalDeviceMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
	m_physicalDeviceMultiviewFeatures.multiview = VK_TRUE;
	m_deviceCreatepNextChain = &m_physicalDeviceMultiviewFeatures;
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

void ApplicationWin::PreparePipeline()
{
	VkPhysicalDeviceFeatures2KHR deviceFeatures2{};
	VkPhysicalDeviceMultiviewFeaturesKHR extFeatures{};

	extFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
	deviceFeatures2.pNext = &extFeatures;
	PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(vkGetInstanceProcAddr(m_instance, "vkGetPhysicalDeviceFeatures2KHR"));
	vkGetPhysicalDeviceFeatures2KHR(m_physicalDevice, &deviceFeatures2);
	std::cout << "Multiview features:" << std::endl;
	std::cout << "\tmultiview = " << extFeatures.multiview << std::endl;
	std::cout << "\tmultiviewGeometryShader = " << extFeatures.multiviewGeometryShader << std::endl;
	std::cout << "\tmultiviewTessellationShader = " << extFeatures.multiviewTessellationShader << std::endl;
	std::cout << std::endl;

	VkPhysicalDeviceProperties2KHR deviceProps2{};
	VkPhysicalDeviceMultiviewPropertiesKHR extProps{};
	extProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHR;
	deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	deviceProps2.pNext = &extProps;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetInstanceProcAddr(m_instance, "vkGetPhysicalDeviceProperties2KHR"));
	vkGetPhysicalDeviceProperties2KHR(m_physicalDevice, &deviceProps2);
	std::cout << "Multiview properties:" << std::endl;
	std::cout << "\tmaxMultiviewViewCount = " << extProps.maxMultiviewViewCount << std::endl;
	std::cout << "\tmaxMultiviewInstanceIndex = " << extProps.maxMultiviewInstanceIndex << std::endl;

	/*
	*	创建图形Pipeline
	*/

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = Render::Vulkan::Initializer::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI = Render::Vulkan::Initializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	VkPipelineColorBlendAttachmentState blendAttachmentState = Render::Vulkan::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI = Render::Vulkan::Initializer::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = Render::Vulkan::Initializer::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportStateCI = Render::Vulkan::Initializer::PipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleStateCI = Render::Vulkan::Initializer::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCI = Render::Vulkan::Initializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);
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

#if defined __ANDROID__
	shaderStages[0] = LoadShader("shaders/glsl/draw_multview/draw_multview.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/glsl/draw_multview/draw_multview.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
	shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_multview/draw_multview.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_multview/draw_multview.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif

	VkGraphicsPipelineCreateInfo pipelineCI = Render::Vulkan::Initializer::PipelineCreateInfo(m_pipelineLayout, m_multivewPass.renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &colorBlendStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = &vertexInputState;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &m_pipeline));

	float multiviewArrayLayer = 0.0f;

	VkSpecializationMapEntry specializationMapEntry{ 0, 0, sizeof(float) };

	VkSpecializationInfo specializationInfo{};
	specializationInfo.dataSize = sizeof(float);
	specializationInfo.mapEntryCount = 1;
	specializationInfo.pMapEntries = &specializationMapEntry;
	specializationInfo.pData = &multiviewArrayLayer;

	rasterizationStateCI.cullMode = VK_CULL_MODE_FRONT_BIT;

	//2个视角
	for (uint32_t i = 0; i < 2; i++) {
#if defined __ANDROID__
		shaderStages[0] = LoadShader("shaders/glsl/draw_multview/draw_display.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = LoadShader("shaders/glsl/draw_multview/draw_display.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
		shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_multview/draw_display.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_multview/draw_display.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif
		shaderStages[1].pSpecializationInfo = &specializationInfo;
		multiviewArrayLayer = (float)i;
		VkPipelineVertexInputStateCreateInfo emptyInputState = Render::Vulkan::Initializer::PipelineVertexInputStateCreateInfo();
		pipelineCI.pVertexInputState = &emptyInputState;
		pipelineCI.layout = m_pipelineLayout;
		pipelineCI.renderPass = m_renderPass;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &viewDisplayPipelines[i]));
	}

}

void ApplicationWin::PrepareUniformBuffer()
{
	for (auto& item : m_uniformBuffers)
	{
		VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &item, sizeof(UniformData), &m_uniformData));
		VK_CHECK_RESULT(item.Map());
	}
}

void ApplicationWin::BuildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];

	VkCommandBufferBeginInfo cmdBufInfo =  Render::Vulkan::Initializer::CommandBufferBeginInfo();


	VkClearValue clearValue[2]{};
	clearValue[0].color = VkClearColorValue{0.2,0.2,0.4,1};
	clearValue[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = Render::Vulkan::Initializer::RenderPassBeginInfo();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	// Update the layered multiview image attachment with the scene from two different viewpors
	{
		renderPassBeginInfo.renderPass = m_multivewPass.renderPass;
		renderPassBeginInfo.framebuffer = m_multivewPass.frameBuffer;

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_currentBuffer], 0, nullptr);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		m_model.Draw(cmdBuffer);

		vkCmdEndRenderPass(cmdBuffer);
	}

	// Display the multiview images
	{
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentImageIndex];

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width / 2.0f, (float)height, 0.0f, 1.0f);
		VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width / 2, height, 0, 0);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_currentBuffer], 0, nullptr);

		// Left eye
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, viewDisplayPipelines[0]);
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

		// Right eye
		viewport.x = (float)width / 2;
		scissor.offset.x = width / 2;
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, viewDisplayPipelines[1]);
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

		DrawUI(cmdBuffer);

		vkCmdEndRenderPass(cmdBuffer);
	}

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
#if defined (__ANDROID__)
	LoadAsset("mesh/room/sampleroom.gltf"); // 加载图片
#else
	LoadAsset("./Asset/mesh/room/sampleroom.gltf"); // 加载图片
#endif
	PrepareMultView();
	PrepareUniformBuffer();
	PrepareDescriptor();
	CreateDescriptorPool();
	PreparePipeline();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{
	// Matrices for the two viewports
// See http://paulbourke.net/stereographics/stereorender/

	float eyeSeparation = 0.08f;
	const float focalLength = 0.5f;
	const float fov = 90.0f;
	const float zNear = 0.1f;
	const float zFar = 256.0f;

// Calculate some variables
	float aspectRatio = (float)(width * 0.5f) / (float)height;
	float wd2 = 0.1 * tan(glm::radians(fov / 2.0f));
	float ndfl = zNear / focalLength;
	float left, right;
	float top = wd2;
	float bottom = -wd2;

	glm::vec3 camFront;
	camFront.x = -cos(glm::radians(m_camera.rotation.x)) * sin(glm::radians(m_camera.rotation.y));
	camFront.y = sin(glm::radians(m_camera.rotation.x));
	camFront.z = cos(glm::radians(m_camera.rotation.x)) * cos(glm::radians(m_camera.rotation.y));
	camFront = glm::normalize(camFront);
	glm::vec3 camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));

	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(m_camera.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_camera.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_camera.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	// Left eye
	left = -aspectRatio * wd2 - 0.5f * eyeSeparation * ndfl;
	right = aspectRatio * wd2 - 0.5f * eyeSeparation * ndfl;

	transM = glm::translate(glm::mat4(1.0f), m_camera.position - camRight * (eyeSeparation / 2.0f));

	m_uniformData.projection[0] = glm::frustum(left, right, bottom, top, zNear, zFar);
	m_uniformData.modelview[0] = rotM * transM;

	// Right eye
	left = -aspectRatio * wd2 + 0.5f * eyeSeparation * ndfl;
	right = aspectRatio * wd2 + 0.5f * eyeSeparation * ndfl;

	transM = glm::translate(glm::mat4(1.0f), m_camera.position + camRight * (eyeSeparation / 2.0f));

	m_uniformData.projection[1] = glm::frustum(left, right, bottom, top, zNear, zFar);
	m_uniformData.modelview[1] = rotM * transM;

	memcpy(m_uniformBuffers[m_currentBuffer].mapped, &m_uniformData, sizeof(UniformData));
}

void ApplicationWin::PrepareMultView()
{
	int multviewLayerCount = 2;

	//深度模板 Attachment
	{
		VkImageCreateInfo imageCI = Render::Vulkan::Initializer::ImageCreateInfo();
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = m_depthFormat;
		imageCI.extent = { width, height, 1 };
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = multviewLayerCount;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCI.flags = 0;
		VK_CHECK_RESULT(vkCreateImage(m_device, &imageCI, nullptr, &m_multivewPass.depth.image));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_device, m_multivewPass.depth.image, &memReqs);

		VkMemoryAllocateInfo memAllocInfo = Render::Vulkan::Initializer::MemoryAllocInfo();
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkImageViewCreateInfo depthStencilView = {};
		depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.pNext = NULL;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		depthStencilView.format = m_depthFormat;
		depthStencilView.flags = 0;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
		{
			depthStencilView.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = multviewLayerCount;
		depthStencilView.image = m_multivewPass.depth.image;

		VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAllocInfo, nullptr, &m_multivewPass.depth.memory));
		VK_CHECK_RESULT(vkBindImageMemory(m_device, m_multivewPass.depth.image, m_multivewPass.depth.memory, 0));
		VK_CHECK_RESULT(vkCreateImageView(m_device, &depthStencilView, nullptr, &m_multivewPass.depth.view));
	}

	//颜色 Attachment
	{
		VkImageCreateInfo imageCI = Render::Vulkan::Initializer::ImageCreateInfo();
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = m_swapChain.colorFormat;
		imageCI.extent = { width, height, 1 };
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = multviewLayerCount;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VK_CHECK_RESULT(vkCreateImage(m_device, &imageCI, nullptr, &m_multivewPass.color.image));

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_device, m_multivewPass.color.image, &memReqs);

		VkMemoryAllocateInfo memoryAllocInfo = Render::Vulkan::Initializer::MemoryAllocInfo();
		memoryAllocInfo.allocationSize = memReqs.size;
		memoryAllocInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(m_device, &memoryAllocInfo, nullptr, &m_multivewPass.color.memory));
		VK_CHECK_RESULT(vkBindImageMemory(m_device, m_multivewPass.color.image, m_multivewPass.color.memory, 0));

		VkImageViewCreateInfo imageViewCI = Render::Vulkan::Initializer::ImageViewCreateInfo();
		imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		imageViewCI.format = m_swapChain.colorFormat;
		imageViewCI.flags = 0;
		imageViewCI.subresourceRange = {};
		imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.levelCount = 1;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = multviewLayerCount;
		imageViewCI.image = m_multivewPass.color.image;
		VK_CHECK_RESULT(vkCreateImageView(m_device, &imageViewCI, nullptr, &m_multivewPass.color.view));

		// Create sampler to sample from the attachment in the fragment shader
		VkSamplerCreateInfo samplerCI = Render::Vulkan::Initializer::SamplerCreateInfo();
		samplerCI.magFilter = VK_FILTER_NEAREST;
		samplerCI.minFilter = VK_FILTER_NEAREST;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = samplerCI.addressModeU;
		samplerCI.addressModeW = samplerCI.addressModeU;
		samplerCI.mipLodBias = 0.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 1.0f;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(m_device, &samplerCI, nullptr, &m_multivewPass.sampler));

		// Fill a descriptor for later use in a descriptor set
		m_multivewPass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_multivewPass.descriptor.imageView = m_multivewPass.color.view;
		m_multivewPass.descriptor.sampler = m_multivewPass.sampler;
	}

	//创建RenderPass
	{
		std::array<VkAttachmentDescription, 2> attachments = {};
		//颜色缓冲区
		attachments[0].format = m_swapChain.colorFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;// 渲染开始时清除深度值
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;// 渲染结束后保存深度值
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;// 清除模板值（如果支持）
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;// 不保存模板数据
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//深度缓冲区
		attachments[1].format = m_depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;//保存
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = {};
		colorReference.attachment = 0;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 1;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		std::array<VkSubpassDependency, 3> dependencies{};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		dependencies[0].dependencyFlags = 0;

		dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].dstSubpass = 0;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[2].srcSubpass = 0;
		dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassCI.pAttachments = attachments.data();
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCI.pDependencies = dependencies.data();

		// 视图掩码：指定哪些视图将被渲染
		// 0b00000011 表示视图 0 和视图 1（最低两位为 1）
		const uint32_t viewMask = 0b00000011;

		// 相关性掩码：指定视图之间的相关性，用于优化
		// 同样 0b00000011 表示视图 0 和 1 是相关的
		const uint32_t correlationMask = 0b00000011;

		VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
		renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
		renderPassMultiviewCI.subpassCount = 1;
		renderPassMultiviewCI.pViewMasks = &viewMask;
		renderPassMultiviewCI.correlationMaskCount = 1;
		renderPassMultiviewCI.pCorrelationMasks = &correlationMask;

		renderPassCI.pNext = &renderPassMultiviewCI;

		VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassCI, nullptr, &m_multivewPass.renderPass));
	}

	//创建FrameBuffer
	{
		VkImageView attachments[2];
		attachments[0] = m_multivewPass.color.view;
		attachments[1] = m_multivewPass.depth.view;

		VkFramebufferCreateInfo framebufferCI = Render::Vulkan::Initializer::FramebufferCreateInfo();
		framebufferCI.renderPass = m_multivewPass.renderPass;
		framebufferCI.attachmentCount = 2;
		framebufferCI.pAttachments = attachments;
		framebufferCI.width = width;
		framebufferCI.height = height;
		framebufferCI.layers = 1;
		VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &framebufferCI, nullptr, &m_multivewPass.frameBuffer));
	}
}

void ApplicationWin::PrepareDescriptor()
{
	//pool
	std::vector<VkDescriptorPoolSize> poolSizes
	{
		Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT),
		Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
	};
	
	VkDescriptorPoolCreateInfo descriptorPoolInfo = Render::Vulkan::Initializer::DescriptorPoolCreateInfo(static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), MAX_FRAMES_IN_FLIGHT);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
			Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = Render::Vulkan::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayout, nullptr, &m_descriptorSetLayout));
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Render::Vulkan::Initializer::PipelineLayoutCreateInfo(&m_descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));

	UpdateDescriptors();
}

void ApplicationWin::UpdateDescriptors()
{
	VkDescriptorSetAllocateInfo allocInfo = Render::Vulkan::Initializer::DescriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout, 1);
	for (int i = 0;i < m_uniformBuffers.size();i++)
	{
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSets[i]));
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSets[i],VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0 ,&m_uniformBuffers[i].descriptor),
			Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSets[i],VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 ,&m_multivewPass.descriptor),
		};
		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
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

void ApplicationWin::LoadAsset(std::string fileNamePath)
{
	m_model.LoadFromFile(fileNamePath, vulkanDevice, m_queue, VkModel::FileLoadingFlags::PreTransformVertices
		| VkModel::FileLoadingFlags::PreMultiplyVertexColors | VkModel::FileLoadingFlags::FlipY);
}