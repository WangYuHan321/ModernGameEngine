#include "Render/Vulkan/ApplicationBase.h"

#define OBJECT_INSTANCE_COUNT 2048

struct ProjectBGTexture{
	Render::Vulkan::VulkanTexture2DArray plants;
	Render::Vulkan::VulkanTexture2D ground;
};

struct ProjectModel {
	Render::Vulkan::VkModel plants;
	Render::Vulkan::VkModel ground;
	Render::Vulkan::VkModel skySphere;
};

struct ProjectPipeline
{
	VkPipeline plants{ VK_NULL_HANDLE };
	VkPipeline ground{ VK_NULL_HANDLE };
	VkPipeline skySphere{ VK_NULL_HANDLE };
};


struct UniformData
{
	glm::mat4 projection;
	glm::mat4 view;
};

struct InstanceData
{
	glm::vec3 pos;
	glm::vec3 rot;
	float scale;
	uint32_t texIndex;
};

class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin() override;
	
private:

	ProjectBGTexture m_texture;
	ProjectModel m_model;
	ProjectPipeline m_pipeline;
	InstanceData m_instanceData;

	Render::Vulkan::Buffer m_instanceBuffer;
	std::vector<VkDrawIndexedIndirectCommand> m_indirectCommands;
	Render::Vulkan::Buffer m_indirectCommandBuffer;
	uint32_t m_indirectDrawCount{ 0 };
	uint32_t m_objectCount{ 0 };

	UniformData m_uniformData;

	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };

	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };

	VkDescriptorSet m_descriptorSet;
	Render::Vulkan::Buffer m_uniformDataBuffer;
public:
	void DrawUI(const VkCommandBuffer cmdBuffer);
	void SetupDescriptors();
	void UpdateUniformBuffers();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);

	void PreparePipeline();
	void PrepareUniformBuffer();
	void BuildCommandBuffer();
	void PrepareInstanceData();
	void PrepareIndirectCommandBuffer();

	virtual void GetEnabledFeatures();

	void Prepare() override;
	void Render();

	void LoadAsset();
};

