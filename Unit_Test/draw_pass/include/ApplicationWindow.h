#include "Render/Vulkan/ApplicationBase.h"

// Vertex layout used in this example
struct Vertex {
	float position[3];
	float color[3];
};

// Vertex buffer and attributes
struct {
	VkDeviceMemory memory{ VK_NULL_HANDLE }; // Handle to the device memory for this buffer
	VkBuffer buffer{ VK_NULL_HANDLE };		 // Handle to the Vulkan buffer object that the memory is bound to
} vertices;

// Index buffer
struct {
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	VkBuffer buffer{ VK_NULL_HANDLE };
	uint32_t count{ 0 };
} indices;

// Uniform buffer block object
struct UniformBuffer {
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	VkBuffer buffer{ VK_NULL_HANDLE };
	// The descriptor set stores the resources bound to the binding points in a shader
	// It connects the binding points of the different shaders with the buffers and images used for those bindings
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	// We keep a pointer to the mapped buffer, so we can easily update it's contents via a memcpy
	uint8_t* mapped{ nullptr };
};

struct ShaderData {
	glm::mat4 projectionMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
};

class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin();

	void Render();
	
private:
	std::array<UniformBuffer, MAX_FRAMES_IN_FLIGHT> m_uniformBuffers;

	VkPipeline			  m_pipeline;
	VkPipelineLayout	  m_pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };

	std::vector<VkSemaphore> m_presentCompleteSemphores{};
	std::vector<VkSemaphore> m_renderCompleteSemphores{};

	VkCommandPool m_commandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers{};
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_waitFences{};

	std::vector<VkSemaphore> m_presentCompleteSemaphores{};
	std::vector<VkSemaphore> m_renderCompleteSemaphores{};

	uint32_t m_currentFrame{ 0 };

private:
	void CreateSynchronizationPrimitives();

};

