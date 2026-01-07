#include "ApplicationWindow.h"

#define OBJECT_NUM 512

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

void ApplicationWin::PrepareGraphicsPipeline()
{

}

void ThreadRenderCode(uint32_t threadIndex, uint32_t cmdBufferIndex, VkCommandBufferInheritanceInfo inheritanceInfo)
{

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





	}

}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	LoadAsset("./Asset/mesh/Sponza/Sponza.gltf"); // 加载图片
	PrepareMulthreadRenderer();
	CreateDescriptorPool();
	PrepareGraphicsPipeline();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{

}

void ApplicationWin::Render()
{
}

void ApplicationWin::LoadAsset(std::string fileNamePath)
{

}