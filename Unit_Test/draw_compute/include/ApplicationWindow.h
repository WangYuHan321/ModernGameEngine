#include "Render/Vulkan/ApplicationBase.h"

// Vertex layout used in this example
struct Vertex {
	float position[3];
	float uv[2];
};

struct Graphics
{
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	struct DescriptorSets
	{
		VkDescriptorSet preCompute{ VK_NULL_HANDLE };
		VkDescriptorSet postCompute{ VK_NULL_HANDLE };
	};
	std::array<DescriptorSets, MAX_FRAMES_IN_FLIGHT> descriptorSets;
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline pipeline{ VK_NULL_HANDLE };

	struct UniformData
	{
		glm::mat4 projection;
		glm::mat4 modelView;
	}uniformData;
	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers;
};

struct Compute
{
	VkQueue queue{ VK_NULL_HANDLE };
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> fences;
	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	std::vector<VkPipeline> pipelines{};
	int32_t pipelineIndex{ 0 };
};

class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin() override;
	
private:
	
	Render::Vulkan::VulkanTexture2D m_textureColorMap;
	Render::Vulkan::VulkanTexture2D m_storageImage;

	Render::Vulkan::Buffer m_vertexBuffer;
	Render::Vulkan::Buffer m_indexBuffer;
	uint32_t m_indexCount = 0;
	uint32_t m_vertexBufferSize{ 0 };

	Graphics m_graphics;
	Compute m_compute;

	std::vector<std::string> m_filterName = { "sharpen" };

	uint32_t m_currentFrame{ 0 };

public:
	void CreateQuad();
	void CreateDescriptorPool();
	void UpdateUniformBuffers();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);

	void PrepareGraphicsPipeline();
	void PrepareComputePipeline();
	void PrepareUniformBuffer();
	void BuildGraphicsCommandBuffer();
	void BuildComputeCommandBuffer();

	void Prepare() override;
	void Render();
	void LoadAsset();
};

