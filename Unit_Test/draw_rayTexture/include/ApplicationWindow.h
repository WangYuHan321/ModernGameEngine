#include "Render/Vulkan/ApplicationRaytracingBase.h"

struct UniformData {
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
};

class ApplicationWin : public ApplicationRaytracingBase
{
public:

	struct ShaderBindingTables {
		ApplicationRaytracingBase::ShaderBindingTable raygen;
		ApplicationRaytracingBase::ShaderBindingTable miss;
		ApplicationRaytracingBase::ShaderBindingTable hit;
	};


	AccelerationStructure m_bottomLevelAS{};
	AccelerationStructure m_topLevelAS{};

	StorageImage m_storageImage;

	ShaderBindingTables m_shaderBindingTables;

	Render::Vulkan::Buffer m_vertexBuffer;
	Render::Vulkan::Buffer m_indexBuffer;
	uint32_t m_indexCount{ 0 };
	Render::Vulkan::Buffer m_transformBuffer;

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups{};

	UniformData m_uniformData;
	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> m_uniformBuffer;

	Render::Vulkan::VulkanTexture2D m_texture;

	VkPipeline  m_pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSet{};

	ApplicationWin();
	~ApplicationWin() override;

public:

	void GetEnabledFeatures() override;

	void CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure,
		VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);

	void CreateBottomLevelAccelerationStructure();
	void CreateTopLevelAccelerationStructure();

	void LoadAssets();
	void CreateStorageImage();
	void CreateUniformBuffer();
	void CreateRayTracingPipeline();
	void CreateShaderBindingTables();
	void CreateDescriptorSets();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);
	void Prepare() override;
	void UpdateUniformBuffers();
	void BuildCommandBuffer();
	void Render();
};

