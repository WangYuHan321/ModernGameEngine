#include "Render/Vulkan/ApplicationBase.h"

struct UniformData
{

};

struct ProjectModel
{
	GlTFModel skyModel;
	std::vector<GlTFModel> objects;
};

class ApplicationWin : public ApplicationBase
{
public:
	ApplicationWin();
	~ApplicationWin() override;
	
private:

	ProjectModel m_projModel;
	VulkanTextureCubeMap m_texCubeMap;

	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> m_uniformBuffer;
	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSets{};
public:
	void DrawUI(const VkCommandBuffer cmdBuffer);
	void SetupDescriptors();
	void UpdateUniformBuffers();
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);

	void PreparePipeline();
	void PrepareUniformBuffer();
	void BuildCommandBuffer();

	void Prepare() override;
	void Render();

	void LoadAsset();
};

