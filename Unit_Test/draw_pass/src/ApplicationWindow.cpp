#include "ApplicationWindow.h"

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationTraingle ";
}

ApplicationWin::~ApplicationWin()
{
	if (m_device) {
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
		vkDestroyBuffer(m_device, vertices.buffer, nullptr);
		vkFreeMemory(m_device, vertices.memory, nullptr);
		vkDestroyBuffer(m_device, indices.buffer, nullptr);
		vkFreeMemory(m_device, indices.memory, nullptr);
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		for (size_t i = 0; i < m_presentCompleteSemaphores.size(); i++) {
			vkDestroySemaphore(m_device, m_presentCompleteSemaphores[i], nullptr);
		}
		for (size_t i = 0; i < m_renderCompleteSemaphores.size(); i++) {
			vkDestroySemaphore(m_device, m_renderCompleteSemaphores[i], nullptr);
		}
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyFence(m_device, m_waitFences[i], nullptr);
			vkDestroyBuffer(m_device, m_uniformBuffers[i].buffer, nullptr);
			vkFreeMemory(m_device, m_uniformBuffers[i].memory, nullptr);
		}
	}
}

void ApplicationWin::CreateSynchronizationPrimitives()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkFenceCreateInfo fenceCI = Render::Vulkan::Initializer::FenceCreateInfo();
		fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCI, nullptr, &m_waitFences[i]));
	}
	m_presentCompleteSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	for (auto& semaphore : m_presentCompleteSemaphores) {
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
	}
	m_renderCompleteSemaphores.resize(m_swapChain.images.size());
	for (auto& semaphore : m_renderCompleteSemaphores) {
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
	}
}

void ApplicationWin::Render()
{

}