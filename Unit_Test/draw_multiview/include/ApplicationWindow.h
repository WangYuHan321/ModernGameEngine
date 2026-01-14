#include "Render/Vulkan/ApplicationBase.h"

struct FrameBufferAttachment
{
	VkImage image{ VK_NULL_HANDLE };
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	VkImageView view{ VK_NULL_HANDLE };
};

struct MultivewPass {
	FrameBufferAttachment color;
	FrameBufferAttachment depth;
	VkFramebuffer frameBuffer{ VK_NULL_HANDLE };
	VkRenderPass renderPass{ VK_NULL_HANDLE };
	VkDescriptorImageInfo descriptor{ VK_NULL_HANDLE };
	VkSampler sampler{ VK_NULL_HANDLE };
};

struct UniformData
{
	glm::mat4 projection[2];
	glm::mat4 modelview[2];
	glm::vec4 lightPos = glm::vec4(-2.5f, -3.5f, 0.0f, 1.0f);
	float distortionAlpha = 0.2f;
};

class ApplicationWin : public ApplicationBase
{
public:

	VkPhysicalDeviceMultiviewFeaturesKHR m_physicalDeviceMultiviewFeatures{};


	MultivewPass m_multivewPass;

	VkModel m_model;

	VkPipeline m_pipeline{ VK_NULL_HANDLE };
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
	std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSets{};

	VkPipeline viewDisplayPipelines[2]{};

	UniformData m_uniformData;

	std::array<Render::Vulkan::Buffer, MAX_FRAMES_IN_FLIGHT> m_uniformBuffers;

	ApplicationWin();
	~ApplicationWin() override;
	
private:

	std::vector<std::string> m_filterName = { "sharpen" };

public:
	void DrawUI(const VkCommandBuffer cmdBuffer);
	void CreateDescriptorPool();
	void UpdateUniformBuffers();


	void PrepareMultView();
	void PrepareGraphicsPipeline();
	void PrepareUniformBuffer();
	void BuildGraphicsCommandBuffer();

	void Prepare() override;
	void Render();
	void LoadAsset(std::string fileNamePath);
};

