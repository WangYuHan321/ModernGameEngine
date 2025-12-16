#include "Render/Vulkan/ApplicationBase.h"

//Scratch Buffer 是GPU上用于加速结构构建和更新的临时工作内存。它不存储最终结果，只在构建过程中使用。
struct RayTracingScratchBuffer
{
	uint64_t deviceAddress = 0;
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct AccelerationStructure
{
	VkAccelerationStructureKHR handle;
	uint64_t deviceAddress = 0;
	VkDeviceMemory memory;
	VkBuffer buffer;
};

struct UniformData {
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
} ;

struct StorageImage 
{
	VkDeviceMemory memory;
	VkImage image;
	VkImageView view;
	VkFormat format;
};

class ApplicationWin : public ApplicationBase
{
public:
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDevieAddressKHR{ nullptr };
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{ nullptr };
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{ nullptr };
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{ nullptr };
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{ nullptr };
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{ nullptr };
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR{ nullptr };
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR{ nullptr };
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR{ nullptr };
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR{ nullptr };

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

	VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};

	AccelerationStructure m_bottomLevelAS{};
	AccelerationStructure m_topLevelAS{};

	StorageImage m_storageImage;

	Render::Vulkan::Buffer m_vertexBuffer;
	Render::Vulkan::Buffer m_indexBuffer;
	uint32_t m_indexCount{ 0 };
	Render::Vulkan::Buffer m_transformBuffer;

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_shaderGroups{};
	Render::Vulkan::Buffer m_raygenShaderBindingTable;
	Render::Vulkan::Buffer m_missShaderBindingTable;
	Render::Vulkan::Buffer m_hitShaderBindingTable;

	UniformData m_uniformData;
	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> m_uniformBuffer;

	VkPipeline  m_pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSet{};

	ApplicationWin();
	~ApplicationWin() override;
	void GetEnabledExtensions() override;

private:
		uint64_t GetBufferDeviceAddress(VkBuffer buffer);
		void CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure,
			VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
		RayTracingScratchBuffer CreateScratchBuffer(VkDeviceSize size);
		void DeleteScratchBuffer(RayTracingScratchBuffer& scratchBuffer);

public:

	void CreateBottomLevelAccelerationStructure();
	void CreateTopLevelAccelerationStructure();

	void CreateStorageImage();
	void CreateUniformBuffer();
	void CreateRayTracingPipeline();
	void CreateShaderBindingTable();
	void CreateDescriptorSets();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);
	void Prepare() override;
	void UpdateUniformBuffers();
	void BuildCommandBuffer();
	void Render();
};

