#include "ApplicationWindow.h"

// plants.gltf 模型文件可能包含多个mesh
// 每个mesh对应一种植物类型，与纹理数组层对应

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationDrawIndirectDraw ";
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

void ApplicationWin::SetupDescriptors()
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

}


void ApplicationWin::GetEnabledFeatures(){
	// Fill mode non solid is required for wireframe display
	if (m_deviceFeatures.fillModeNonSolid) {
		m_enabledFeatures.fillModeNonSolid = VK_TRUE;
	};
}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	LoadAsset(); // 加载图片
	PrepareUniformBuffer();
	SetupDescriptors();
	PreparePipeline();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{

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
	LoadGlTFFile("model/1.gltf", m_model.plants);
	LoadGlTFFile("model/1.gltf", m_model.ground);
	LoadGlTFFile("model/1.gltf", m_model.skySphere);

	std::vector<std::string> strFileVec;
	m_texture.plants.LoadFromFile(strFileVec, VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue);
	m_texture.ground.LoadFromFile("", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue);
}