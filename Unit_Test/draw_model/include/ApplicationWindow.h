#include "Render/Vulkan/ApplicationBase.h"

// Vertex layout used in this example
struct Vertex {
	float position[3];
	float uv[2];
};

struct UniformData
{
	glm::mat4 projection;
	glm::mat4 model;
	glm::vec4 lightPos = glm::vec4(5, 5, -5, 1);
	glm::vec4 viewPos;
};

struct DescriptorSetLayouts {
	VkDescriptorSetLayout matrices{ VK_NULL_HANDLE };
	VkDescriptorSetLayout textures{ VK_NULL_HANDLE };
};

struct Pipelines
{
	VkPipeline solid{ VK_NULL_HANDLE };
	VkPipeline wireFrame{ VK_NULL_HANDLE };
};

class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin() override;
	
private:

	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	Pipelines m_pipelines;

	UniformData m_uniform;
	DescriptorSetLayouts m_descriptorSetLayout;

	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSet;
	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> m_uniformDataBuffer;

public:
	void DrawUI(const VkCommandBuffer cmdBuffer);
	void CreateDescriptorPool();
	void UpdateUniformBuffers();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);

	void PreparePipeline();
	void PrepareUniformBuffer();
	void BuildCommandBuffer();

	virtual void GetEnabledFeatures();

	void Prepare() override;
	void Render();
	void LoadAsset();
};

