#include "ApplicationWindow.h"

#include <chrono>

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationDeferredLighting ";
	m_camera.type = Camera::CameraType::lookat;
	m_camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
	m_camera.setRotation(glm::vec3(0.0f));
	m_camera.setPerspective(60.0f, (float)width * 0.5f / (float)height, 1.0f, 256.0f);
}

ApplicationWin::~ApplicationWin()
{
	if (m_device) {
		if (offscreenframeBuffers.deferred)
		{
			delete offscreenframeBuffers.deferred;
		}
		if (offscreenframeBuffers.shadow)
		{
			delete offscreenframeBuffers.shadow;
		}
		vkDestroyPipeline(m_device, pipelines.deferred, nullptr);
		vkDestroyPipeline(m_device, pipelines.offscreen, nullptr);
		vkDestroyPipeline(m_device, pipelines.shadowpass, nullptr);
		vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
		for (auto& buffer : uniformBuffers) {
			buffer.offscreen.Destroy();
			buffer.composition.Destroy();
			buffer.shadowGeometryShader.Destroy();
		}
		textures.model.colorMap.Destory();
		textures.model.normalMap.Destory();
		textures.background.colorMap.Destory();
		textures.background.normalMap.Destory();
	}
}

void ApplicationWin::GetEnabledFeatures()
{
	if (m_deviceFeatures.geometryShader)
		m_enabledFeatures.geometryShader = VK_TRUE;
	else
		printf("error dont not support geometry shader !!!\n");

	if (m_deviceFeatures.samplerAnisotropy)
		m_enabledFeatures.samplerAnisotropy = VK_TRUE;

	// Enable texture compression
	if (m_deviceFeatures.textureCompressionBC) {
		m_enabledFeatures.textureCompressionBC = VK_TRUE;
	}
	else if (m_deviceFeatures.textureCompressionASTC_LDR) {
		m_enabledFeatures.textureCompressionASTC_LDR = VK_TRUE;
	}
	else if (m_deviceFeatures.textureCompressionETC2) {
		m_enabledFeatures.textureCompressionETC2 = VK_TRUE;
	}

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


void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	LoadAsset(); // 加载图片
	DeferredSetUp();
	ShadowSetUp();
	InitLights();
	PrepareUniformBuffers();
	SetupDescriptors();
	PreparePipelines();
	prepared = true;
}


void ApplicationWin::ShadowSetUp()
{
	offscreenframeBuffers.shadow = new VulkanFrameBuffer(vulkanDevice);

#if defined(__ANDROID__)
	offscreenframeBuffers.shadow->width = 1024;
	offscreenframeBuffers.shadow->height = 1024;
#else
	offscreenframeBuffers.shadow->width = 2048;
	offscreenframeBuffers.shadow->height = 2048;
#endif

	VkFormat shadowMapFormat;
	VkBool32 validShadowMapFormat = Render::Vulkan::Tool::GetSupportedDepthFormat(m_physicalDevice, &shadowMapFormat);
	assert(validShadowMapFormat);

	Render::Vulkan::AttachmentCreateInfo attachmentInfo = {};
	attachmentInfo.format = shadowMapFormat;
	attachmentInfo.width = offscreenframeBuffers.shadow->width;
	attachmentInfo.height = offscreenframeBuffers.shadow->height;
	attachmentInfo.layerCount = LIGHT_COUNT;
	attachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	offscreenframeBuffers.shadow->AddAttachment(attachmentInfo);

	VK_CHECK_RESULT(offscreenframeBuffers.shadow->CreateSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));

	VK_CHECK_RESULT(offscreenframeBuffers.shadow->CreateRenderPass());
}

void ApplicationWin::DeferredSetUp()
{
	offscreenframeBuffers.deferred = new VulkanFrameBuffer(vulkanDevice);

#if defined(__ANDROID__)
	offscreenframeBuffers.deferred->width = 1024;
	offscreenframeBuffers.deferred->height = 1024;
#else
	offscreenframeBuffers.deferred->width = 2048;
	offscreenframeBuffers.deferred->height = 2048;
#endif

	// 3 color buffer 1 depth buffer
	Render::Vulkan::AttachmentCreateInfo attachmentInfo = {};
	attachmentInfo.width = offscreenframeBuffers.deferred->width;
	attachmentInfo.height = offscreenframeBuffers.deferred->height;
	attachmentInfo.layerCount = 1;
	attachmentInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	
	//Attachment 0 Postion
	attachmentInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	offscreenframeBuffers.deferred->AddAttachment(attachmentInfo);

	//Attachemnt 1 Normal
	attachmentInfo.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	offscreenframeBuffers.deferred->AddAttachment(attachmentInfo);

	//Attachment 2 Albedo 这里可能不正确 UNORM是把RGB当成线性数据了
	attachmentInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	offscreenframeBuffers.deferred->AddAttachment(attachmentInfo);

	// Attachment 3 Depth
	VkFormat attDepthFormat;
	VkBool32 validDepthFormat = Render::Vulkan::Tool::GetSupportedDepthFormat(m_physicalDevice, &attDepthFormat);
	assert(validDepthFormat);
	attachmentInfo.format = attDepthFormat;
	attachmentInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	offscreenframeBuffers.deferred->AddAttachment(attachmentInfo);

	// Create sampler to sample from the color attachments
	VK_CHECK_RESULT(offscreenframeBuffers.deferred->CreateSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));

	// Create default renderpass for the framebuffer
	VK_CHECK_RESULT(offscreenframeBuffers.deferred->CreateRenderPass());
}

void ApplicationWin::SetupDescriptors()
{
	//Pool
	std::vector<VkDescriptorPoolSize>poolSize = {
		Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT * 8),
		Render::Vulkan::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * 16)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = Render::Vulkan::Initializer::DescriptorPoolCreateInfo(poolSize, MAX_FRAMES_IN_FLIGHT * 4);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));

	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0: Vertex shader uniform buffer
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 0),
		// Binding 1: Position texture
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		// Binding 2: Normals texture
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		// Binding 3: Albedo texture
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
		// Binding 4: Fragment shader uniform buffer
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
		// Binding 5: Shadow map
		Render::Vulkan::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = Render::Vulkan::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayout, nullptr, &descriptorSetLayout));

	VkDescriptorImageInfo descriptorPosition = Render::Vulkan::Initializer::DescriptorImageInfo(offscreenframeBuffers.
		deferred->sampler, offscreenframeBuffers.deferred->attachments[0].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo descriptorNormal = Render::Vulkan::Initializer::DescriptorImageInfo(offscreenframeBuffers.
		deferred->sampler, offscreenframeBuffers.deferred->attachments[1].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo descriptorAlbedo = Render::Vulkan::Initializer::DescriptorImageInfo(offscreenframeBuffers.
		deferred->sampler, offscreenframeBuffers.deferred->attachments[2].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo descriptorShadowMap = Render::Vulkan::Initializer::DescriptorImageInfo(offscreenframeBuffers.
		shadow->sampler, offscreenframeBuffers.shadow->attachments[0].imageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

	VkDescriptorSetAllocateInfo allocInfo = Render::Vulkan::Initializer::DescriptorSetAllocateInfo(m_descriptorPool, 
		&descriptorSetLayout, 1);

	for (auto i = 0; i < uniformBuffers.size();i++)
	{
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		// Deferred composition
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSets[i].composition));

		writeDescriptorSets = {
			// Binding 1: World space position texture
				Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].composition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &descriptorPosition),
				// Binding 2: World space normals texture
				Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].composition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &descriptorNormal),
				// Binding 3: Albedo texture
				Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].composition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &descriptorAlbedo),
				// Binding 4: Fragment shader uniform buffer
				Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].composition, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &uniformBuffers[i].composition.descriptor),
				// Binding 5: Shadow map
				Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].composition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &descriptorShadowMap),
		};

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);



		// Offscreen (scene)

		// Model
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSets[i].model));
		writeDescriptorSets = {
			// Binding 0: Vertex shader uniform buffer
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].model, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[i].offscreen.descriptor),
			// Binding 1: Color map
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].model, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.model.colorMap.descirptor),
			// Binding 2: Normal map
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].model, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textures.model.normalMap.descirptor)
		};
		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		// Background
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSets[i].background));
		writeDescriptorSets = {
			// Binding 0: Vertex shader uniform buffer
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].background, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[i].offscreen.descriptor),
			// Binding 1: Color map
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].background, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textures.background.colorMap.descirptor),
			// Binding 2: Normal map
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].background, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textures.background.normalMap.descirptor)
		};
		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

		// Shadow mapping
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSets[i].shadow));
		writeDescriptorSets = {
			// Binding 0: Vertex shader uniform buffer
			Render::Vulkan::Initializer::WriteDescriptorSet(descriptorSets[i].shadow, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[i].shadowGeometryShader.descriptor),
		};

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

}

void ApplicationWin::PreparePipelines()
{
	// Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = Render::Vulkan::Initializer::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	// pipelines
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = Render::Vulkan::Initializer::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = Render::Vulkan::Initializer::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = Render::Vulkan::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = Render::Vulkan::Initializer::PipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = Render::Vulkan::Initializer::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = Render::Vulkan::Initializer::PipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = Render::Vulkan::Initializer::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = Render::Vulkan::Initializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	VkGraphicsPipelineCreateInfo pipelineCI = Render::Vulkan::Initializer::PipelineCreateInfo(pipelineLayout, m_renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

#if defined (__ANDROID__)
    shaderStages[0] = LoadShader("shaders/glsl/draw_deferredLighting/deferred.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = LoadShader("shaders/glsl/draw_deferredLighting/deferred.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
    shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_deferredLighting/deferred.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_deferredLighting/deferred.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif

	VkPipelineVertexInputStateCreateInfo emptyInputState{};
	emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineCI.pVertexInputState = &emptyInputState;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &pipelines.deferred));

	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(VkModel::Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 5> vertexInputAttrs{};
	vertexInputAttrs[0].binding = 0;
	vertexInputAttrs[0].location = 0;
	vertexInputAttrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttrs[0].offset = offsetof(VkModel::Vertex, pos);
	vertexInputAttrs[1].binding = 0;
	vertexInputAttrs[1].location = 1;
	vertexInputAttrs[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttrs[1].offset = offsetof(VkModel::Vertex, uv);
	vertexInputAttrs[2].binding = 0;
	vertexInputAttrs[2].location = 2;
	vertexInputAttrs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttrs[2].offset = offsetof(VkModel::Vertex, color);
	vertexInputAttrs[3].binding = 0;
	vertexInputAttrs[3].location = 3;
	vertexInputAttrs[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttrs[3].offset = offsetof(VkModel::Vertex, normal);
	vertexInputAttrs[4].binding = 0;
	vertexInputAttrs[4].location = 4;
	vertexInputAttrs[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttrs[4].offset = offsetof(VkModel::Vertex, tangent);

	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttrs.size());
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttrs.data();
	pipelineCI.pVertexInputState = &vertexInputState;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

	// Offscreen pipeline
	// Separate render pass
	pipelineCI.renderPass = offscreenframeBuffers.deferred->renderPass;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates =
	{
		Render::Vulkan::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		Render::Vulkan::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		Render::Vulkan::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};
	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

#if defined (__ANDROID__)
    shaderStages[0] = LoadShader("shaders/glsl/draw_deferredLighting/gbuffer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = LoadShader("shaders/glsl/draw_deferredLighting/gbuffer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#else
    shaderStages[0] = LoadShader("./Asset/shader/glsl/draw_deferredLighting/gbuffer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("./Asset/shader/glsl/draw_deferredLighting/gbuffer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
#endif


	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &pipelines.offscreen));

	// Shadow mapping pipeline
	// The shadow mapping pipeline uses geometry shader instancing (invocations layout modifier) to output
	// shadow maps for multiple lights sources into the different shadow map layers in one single render pass
	std::array<VkPipelineShaderStageCreateInfo, 2> shadowStages{};
#if defined (__ANDROID__)
    shadowStages[0] = LoadShader("shaders/glsl/draw_deferredLighting/shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shadowStages[1] = LoadShader("shaders/glsl/draw_deferredLighting/shadow.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);
#else
    shadowStages[0] = LoadShader("./Asset/shader/glsl/draw_deferredLighting/shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shadowStages[1] = LoadShader("./Asset/shader/glsl/draw_deferredLighting/shadow.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);
#endif

	pipelineCI.pStages = shadowStages.data();
	pipelineCI.stageCount = static_cast<uint32_t>(shadowStages.size());

	// Shadow pass doesn't use any color attachments
	colorBlendState.attachmentCount = 0;
	colorBlendState.pAttachments = nullptr;
	// Cull front faces
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	// Enable depth bias
	rasterizationState.depthBiasEnable = VK_TRUE;
	// Add depth bias to dynamic state, so we can change it at runtime
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	dynamicState = Render::Vulkan::Initializer::PipelineDynamicStateCreateInfo(dynamicStateEnables);
	// Reset blend attachment state
	pipelineCI.renderPass = offscreenframeBuffers.shadow->renderPass;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &pipelines.shadowpass));
}

// Prepare and initialize uniform buffer containing shader uniforms
void ApplicationWin::PrepareUniformBuffers()
{
	for (auto& buffer : uniformBuffers) {
		// Offscreen
		VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.offscreen, sizeof(UniformDataOffscreen)));
		VK_CHECK_RESULT(buffer.offscreen.Map());
		// Composition
		VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.composition, sizeof(UniformDataComposition)));
		VK_CHECK_RESULT(buffer.composition.Map());
		// Shadow map
		VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.shadowGeometryShader, sizeof(UniformDataShadows)));
		VK_CHECK_RESULT(buffer.shadowGeometryShader.Map());
	}

	// Setup instanced model positions
	uniformDataOffscreen.instancePos[0] = glm::vec4(0.0f);
	uniformDataOffscreen.instancePos[1] = glm::vec4(-7.0f, 0.0, -4.0f, 0.0f);
	uniformDataOffscreen.instancePos[2] = glm::vec4(4.0f, 0.0, -6.0f, 0.0f);
}

Light InitLight(glm::vec3 pos, glm::vec3 target, glm::vec3 color)
{
	Light light;
	light.position = glm::vec4(pos, 1.0f);
	light.target = glm::vec4(target, 0.0f);
	light.color = glm::vec4(color, 0.0f);
	return light;
}

void ApplicationWin::InitLights()
{
	uniformDataComposition.lights[0] = InitLight(glm::vec3(-14.0f, -0.5f, 15.0f), glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.5f, 0.5f));
	uniformDataComposition.lights[1] = InitLight(glm::vec3(14.0f, -4.0f, 12.0f), glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	uniformDataComposition.lights[2] = InitLight(glm::vec3(0.0f, -10.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
}

// Update deferred composition fragment shader light position and parameters uniform block
void ApplicationWin::UpdateUniformBufferDeferred()
{
	// Move the lights slowly around their initial positions.
	static const auto startTime = std::chrono::steady_clock::now();
	const float elapsedSeconds = std::chrono::duration<float>(
		std::chrono::steady_clock::now() - startTime).count();
	const float angle = elapsedSeconds * glm::radians(10.0f);

	uniformDataComposition.lights[0].position.x = -14.0f + std::sin(angle) * 6.0f;
	uniformDataComposition.lights[0].position.z = 15.0f + std::cos(angle) * 3.0f;

	const float light1Angle = angle * 0.8f + glm::radians(180.0f);
	uniformDataComposition.lights[1].position.x = 14.0f + std::sin(light1Angle) * 4.0f;
	uniformDataComposition.lights[1].position.z = 12.0f + std::cos(light1Angle) * 4.0f;

	const float light2Angle = angle * 0.6f;
	uniformDataComposition.lights[2].position.x = std::sin(light2Angle) * 4.0f;
	uniformDataComposition.lights[2].position.z = 4.0f + std::cos(light2Angle) * 2.0f;

	for (uint32_t i = 0; i < LIGHT_COUNT; i++) {
		// mvp from light's pov (for shadows)
		glm::mat4 shadowProj = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
		glm::mat4 shadowView = glm::lookAt(glm::vec3(uniformDataComposition.lights[i].position), glm::vec3(uniformDataComposition.lights[i].target), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 shadowModel = glm::mat4(1.0f);

		uniformDataShadows.mvp[i] = shadowProj * shadowView * shadowModel;
		uniformDataComposition.lights[i].viewMatrix = uniformDataShadows.mvp[i];
	}

	memcpy(uniformDataShadows.instancePos, uniformDataOffscreen.instancePos, sizeof(UniformDataOffscreen::instancePos));
	memcpy(uniformBuffers[m_currentBuffer].shadowGeometryShader.mapped, &uniformDataShadows, sizeof(UniformDataShadows));

	uniformDataComposition.viewPos = glm::vec4(m_camera.position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);;
	uniformDataComposition.debugDisplayTarget = debugDisplayTarget;
	memcpy(uniformBuffers[m_currentBuffer].composition.mapped, &uniformDataComposition, sizeof(uniformDataComposition));
}

void ApplicationWin::UpdateUniformBufferOffscreen()
{
	uniformDataOffscreen.projection = m_camera.matrices.perspective;
	uniformDataOffscreen.view = m_camera.matrices.view;
	uniformDataOffscreen.model = glm::mat4(1.0f);
	memcpy(uniformBuffers[m_currentBuffer].offscreen.mapped, &uniformDataOffscreen, sizeof(uniformDataOffscreen));
}

void ApplicationWin::BuildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];
	VkCommandBufferBeginInfo cmdBufInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	//First render pass : shadow map generation
	{
		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = Render::Vulkan::Initializer::RenderPassBeginInfo();
		renderPassBeginInfo.renderPass = offscreenframeBuffers.shadow->renderPass;
		renderPassBeginInfo.framebuffer = offscreenframeBuffers.shadow->frameBuffer;
		renderPassBeginInfo.renderArea = { 0, 0, offscreenframeBuffers.shadow->width, offscreenframeBuffers.shadow->height };
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)offscreenframeBuffers.shadow->width, (float)offscreenframeBuffers.shadow->height, 0.0f, 1.0f);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(offscreenframeBuffers.shadow->width, offscreenframeBuffers.shadow->height, 0, 0);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
		// Set depth bias to avoid shadow artefacts from self-shadowing (aka "Polygon offset")
		vkCmdSetDepthBias(cmdBuffer, depthBiasConstant, 0.0f, depthBiasSlope);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.shadowpass);
		RenderScene(cmdBuffer, true);
		vkCmdEndRenderPass(cmdBuffer);
	}

	// Second render pass: Composition
	// Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies

	{
		// Clear values for all attachments written in the fragment shader
		VkRenderPassBeginInfo renderPassBeginInfo = Render::Vulkan::Initializer::RenderPassBeginInfo();
		std::array<VkClearValue, 4> clearValues{};
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[3].depthStencil = { 1.0f, 0 };

		renderPassBeginInfo.renderPass = offscreenframeBuffers.deferred->renderPass;
		renderPassBeginInfo.framebuffer = offscreenframeBuffers.deferred->frameBuffer;
		renderPassBeginInfo.renderArea.extent.width = offscreenframeBuffers.deferred->width;
		renderPassBeginInfo.renderArea.extent.height = offscreenframeBuffers.deferred->height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)offscreenframeBuffers.deferred->width, (float)offscreenframeBuffers.deferred->height, 0.0f, 1.0f);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(offscreenframeBuffers.deferred->width, offscreenframeBuffers.deferred->height, 0, 0);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.offscreen);
		RenderScene(cmdBuffer, false);
		vkCmdEndRenderPass(cmdBuffer);
	}

	// Third render pass: Composition
	// Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies
	{
		VkClearValue clearValues[2]{};
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = Render::Vulkan::Initializer::RenderPassBeginInfo();
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentImageIndex];

		vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[m_currentBuffer].composition, 0, nullptr);
		// Final composition as full screen quad
		// Note: Also used for debug display if debugDisplayTarget > 0
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.deferred);
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
		DrawUI(cmdBuffer);
		vkCmdEndRenderPass(cmdBuffer);
	}

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

}

void ApplicationWin::RenderScene(VkCommandBuffer cmdBuffer, bool shadow)
{
	auto& currentDescriptorSet = descriptorSets[m_currentBuffer];

	// Background
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, shadow ? &currentDescriptorSet.shadow : &currentDescriptorSet.background, 0, nullptr);
	models.background.Draw(cmdBuffer);

	// Objects
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, shadow ? &currentDescriptorSet.shadow : &currentDescriptorSet.model, 0, nullptr);
	models.model.BindBuffers(cmdBuffer);
	vkCmdDrawIndexed(cmdBuffer, models.model.indices.count, 3, 0, 0, 0);
}

void ApplicationWin::Render()
{
	if (!prepared)
		return;

	//Vulkan CONCURRENT 模式只是解决了资源的"访问权限"问题，完全不处理"执行同步"！
	// 
	// 屏障是GPU内部的同步：确保命令缓冲区内的执行顺序
	// Fence / 信号量是CPU - GPU间的同步：确保命令缓冲区间的执行顺序
	//屏障的前提：相关的命令缓冲区必须已经提交并在执行中
	//结论：你不能移除 compute fence 的等待。内存屏障需要计算命令已经开始执行才能正常工作。如果移除 fence 等待，会导致未定义行为（数据竞争、图像撕裂或验证层错误）。
	//所以这里必须 等待计算队列完成

	vkWaitForFences(m_device, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &m_waitFences[m_currentBuffer]);
	m_swapChain.AcquireNextImage(m_presentCompleteSemaphores[m_currentBuffer], m_currentImageIndex);

	UpdateUniformBufferDeferred();
	UpdateUniformBufferOffscreen();
	BuildCommandBuffer();

	ApplicationBase::SubmitFrame(false);
}

void ApplicationWin::LoadAsset()
{
	const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	const uint32_t glTFLoadingFlags = VkModel::FileLoadingFlags::PreTransformVertices
		| VkModel::FileLoadingFlags::PreMultiplyVertexColors | VkModel::FileLoadingFlags::FlipY;

#if defined(__ANDROID__)

    models.model.LoadFromFile("mesh/armor/armor.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
    models.background.LoadFromFile("mesh/armor/deferred_box.gltf", vulkanDevice, m_queue, glTFLoadingFlags);

    textures.model.colorMap.LoadFromFile("texture/floor.png", format, vulkanDevice, m_queue,
                                         VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);
    textures.model.normalMap.LoadFromFile("texture/floor.png", format, vulkanDevice, m_queue,
                                          VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);
    textures.background.colorMap.LoadFromFile("texture/floor.png", format, vulkanDevice, m_queue,
                                              VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);
    textures.background.normalMap.LoadFromFile("texture/floor.png", format, vulkanDevice, m_queue,
                                               VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);

#else

	models.model.LoadFromFile("./Asset/mesh/armor/armor.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
	models.background.LoadFromFile("./Asset/mesh/armor/deferred_box.gltf", vulkanDevice, m_queue, glTFLoadingFlags);
	
	textures.model.colorMap.LoadFromFile("./Asset/texture/floor.png", format, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);
	textures.model.normalMap.LoadFromFile("./Asset/texture/floor.png", format, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);
	textures.background.colorMap.LoadFromFile("./Asset/texture/floor.png", format, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);
	textures.background.normalMap.LoadFromFile("./Asset/texture/floor.png", format, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_GENERAL);


#endif

}