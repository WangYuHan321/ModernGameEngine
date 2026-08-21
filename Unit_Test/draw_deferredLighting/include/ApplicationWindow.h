#include "Render/Vulkan/ApplicationBase.h"

#define LIGHT_COUNT 3

struct Light
{
	glm::vec4 position;
	glm::vec4 target;
	glm::vec4 color;
	glm::mat4 viewMatrix;
};

class ApplicationWin : public ApplicationBase
{
public:

	int32_t debugDisplayTarget{ 0 };
	bool enableShadows = true;

	float zNear = 0.1f;
	float zFar = 64.0f;
	float lightFOV = 100.0f;

	float depthBiasConstant = 1.25f;
	float depthBiasSlope = 1.75f;

	//场景模型  纹理 和 背景 纹理
	struct 
	{
		struct
		{
			VulkanTexture2D colorMap;
			VulkanTexture2D normalMap;
		}model;
		struct
		{
			VulkanTexture2D colorMap;
			VulkanTexture2D normalMap;
		} background;
	}textures{};

	struct 
	{
		VkModel model;
		VkModel background;
	} models;

	struct UniformDataOffscreen 
	{
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::vec4 instancePos[3];
		int layer{0};
	} uniformDataOffscreen;


	struct UniformDataShadows {
		glm::mat4 mvp[LIGHT_COUNT];
		glm::vec4 instancePos[3];
	} uniformDataShadows;

	struct UniformDataComposition
	{
		glm::vec4 viewPos;
		Light lights[LIGHT_COUNT];
		uint32_t useShadows = 1;
		int32_t debugDisplayTarget = 0;
	} uniformDataComposition;

	//GPU UniformBuffer
	struct UniformBuffers
	{
		Render::Vulkan::Buffer offscreen;
		Render::Vulkan::Buffer composition;
		Render::Vulkan::Buffer shadowGeometryShader;
	};

	std::array<UniformBuffers, MAX_FRAMES_IN_FLIGHT> uniformBuffers;

	VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	struct
	{
		VkPipeline deferred{ VK_NULL_HANDLE };
		VkPipeline offscreen{ VK_NULL_HANDLE };
		VkPipeline shadowpass{ VK_NULL_HANDLE };
	}pipelines;

	VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
	struct DescriptorSets
	{
		VkDescriptorSet model{ VK_NULL_HANDLE };
		VkDescriptorSet background{ VK_NULL_HANDLE };
		VkDescriptorSet composition{ VK_NULL_HANDLE };
		VkDescriptorSet shadow{ VK_NULL_HANDLE };
	};

	std::array<DescriptorSets, MAX_FRAMES_IN_FLIGHT> descriptorSets;

	struct
	{
		Render::Vulkan::VulkanFrameBuffer* deferred;
		Render::Vulkan::VulkanFrameBuffer* shadow;
	}offscreenframeBuffers{};

	ApplicationWin();
	~ApplicationWin() override;
	

public:

	VkPipelineVertexInputStateCreateInfo* GetPipelineVertexInputState();

	virtual void GetEnabledFeatures() override;

	void DrawUI(const VkCommandBuffer cmdBuffer);
	void CreateDescriptorPool();
	void UpdateUniformBuffers();

	void ShadowSetUp();
	void DeferredSetUp();
	void SetupDescriptors();
	void PreparePipelines();
	void PrepareUniformBuffers();
	void InitLights();
	void UpdateUniformBufferDeferred();
	void UpdateUniformBufferOffscreen();
	void BuildCommandBuffer();
	void RenderScene(VkCommandBuffer cmdBuffer, bool useDynamicRendering = false);
	
	VkShaderModule LoadSPIRVShader(const std::string& filename);

	void Prepare() override;
	void Render();
	void LoadAsset();
};

