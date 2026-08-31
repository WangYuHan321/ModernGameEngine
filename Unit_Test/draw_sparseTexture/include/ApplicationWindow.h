#include "Render/Vulkan/ApplicationBase.h"

struct VirtualTexturePage
{
	VkOffset3D offset;
	VkExtent3D extent;
	VkSparseImageMemoryBind imageMemoryBind;
	VkDeviceSize size;
	uint32_t mipLevel;
	uint32_t layer;
	uint32_t index;
	bool del;

	VirtualTexturePage();



};



class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin() override;
	
private:
	bool m_wireFrame = false;

	GlTFModel m_glTFModel;
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
	virtual void OnUpdateUIOverlay(Render::Vulkan::UIOverlay* overlay);

	void Prepare() override;
	void Render();
	void LoadAsset();
};

