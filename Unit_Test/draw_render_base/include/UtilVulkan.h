//--------------------------------------------------------
// Write by WangYuHan 2025 09.21
// This is a demo to lean how to write vulkan
//--------------------------------------------------------

#pragma once

#include <array>
#include <functional>
#include <vector>
#include <vulkan/vulkan.h>

#include "glslang_c_interface.h"

namespace vkRenderer
{

	#define VK_CHECK(value) vkRenderer::CHECK(value == VK_SUCCESS, __FILE__, __LINE__);
	#define VK_CHECK_RET(value) if ( value != VK_SUCCESS ) { vkRenderer::CHECK(false, __FILE__, __LINE__); return value; }
	#define BL_CHECK(value) vkRenderer::CHECK(value, __FILE__, __LINE__);

	struct VulkanInstance final
	{
		VkInstance instance;
		VkSurfaceKHR surface;
		VkDebugUtilsMessengerEXT messenger;
		VkDebugReportCallbackEXT reportCallback;
	};

	struct VulkanRenderDevice final
	{
		uint32_t frameBufferWidth;
		uint32_t frameBufferHeight;

		VkDevice device;
		VkQueue graphicsQueue;
		VkPhysicalDevice physicalDevice;

		uint32_t graphicsFamily; // 图行家族索引

		VkSwapchainKHR swapchain;
		VkSemaphore semaphore;
		VkSemaphore renderSemaphore;

		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageView;

		bool useCompute = false;

		uint32_t computeFamily;
		VkQueue computeQueue;

		std::vector<uint32_t> deviceQueueIndices;
		std::vector<VkQueue> deviceQueues;

		VkCommandBuffer computeCommandBuffer;
		VkCommandPool computeCommandPool;
	};

	//为了获取vulkan的Context内容
	struct VulkanContextFeatures
	{
		bool supportScreenshots_ = false;

		bool geometryShader_ = true;
		bool tessellationShader_ = false;

		bool vertexPipelineStoresAndAtomics_ = false;
		bool fragmentStoresAndAtomics_ = false;
	};

	struct VulkanContextCreator
	{
		VulkanContextCreator() = default;

		VulkanContextCreator(VulkanInstance& vk, VulkanRenderDevice& dev, void* window, uint32_t width, uint32_t height, const VulkanContextFeatures& ctxFeature = VulkanContextFeatures());
		~VulkanContextCreator();

		VulkanInstance& instance;
		VulkanRenderDevice& vkDevice;
	};

	struct SwapchainSupportDetails final
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct ShaderModule final
	{
		std::vector<unsigned int> SPIRV;
		VkShaderModule shaderModule = nullptr;
	};

	struct VulkanBuffer
	{
		VkBuffer buffer;
		VkDeviceSize size;
		VkDeviceMemory memory;

		//CPU 地址
		void* ptr;
	};

	struct VulkanImage final
	{
		VkImage image = nullptr;
		VkDeviceMemory imageMemory = nullptr;
		VkImageView imageView = nullptr;
	};

	struct VulkanTexture final
	{
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		VkFormat format;

		VulkanImage image;
		VkSampler sampler;

		//目标布局
		VkImageLayout desiredLayout;
	};

	void CHECK(bool check, const char* fileName, int inlineNumber);

	bool SetupDebugCallbacks(VkInstance instance, VkDebugUtilsMessengerEXT* messenger, VkDebugReportCallbackEXT* reportCallback);

	VkShaderStageFlagBits GlslangShaderStageToVulkan(glslang_stage_t sh);

	glslang_stage_t GlslangShaderStageFromFileName(const char* fileName);

	size_t CompileShaderFile(const char* file, ShaderModule& shaderModule);

	VkResult CreateShaderModule(VkDevice device, ShaderModule* shader, const char* fileName);

	inline VkPipelineShaderStageCreateInfo ShaderStageInfo(VkShaderStageFlagBits shadrStage, ShaderModule& module, const char*  entryPoint )
	{
		return VkPipelineShaderStageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = shadrStage,
			.module = module.shaderModule,
			.pName = entryPoint,
			.pSpecializationInfo = nullptr
		};
	}

	inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1)
	{
		return VkDescriptorSetLayoutBinding{
			.binding = binding,
			.descriptorType = descriptorType,
			.descriptorCount = descriptorCount,
			.stageFlags = stageFlags,
			.pImmutableSamplers = nullptr
		};
	}

	void CreateInstance(VkInstance* instance);
	
	VkResult CreateDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures devFeature, uint32_t graphicsFamily, VkDevice* device);

	VkResult CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t graphicsFamily, uint32_t width, uint32_t height, VkSwapchainKHR* swapchain, bool supportScreenshots = false);

	size_t CreateSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews);

	VkResult CreateSemaphore(VkDevice device, VkSemaphore* outSemaphore);

	bool CreateTextureSampler(VkDevice device, VkSampler* sampler, VkFilter minFilter = VK_FILTER_LINEAR, VkFilter maxFilter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

	bool CreateDescriptorPool(VulkanRenderDevice& vkDev, uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount, VkDescriptorPool* descriptorPool);

	bool IsDeviceSuitable(VkPhysicalDevice device);

	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	uint32_t ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

	VkResult FindSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice);

	uint32_t FindQueueFamilies(VkPhysicalDevice device, VkQueueFlags desiredFlags);

	VkFormat FindSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	uint32_t FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkFormat FindDepthFormat(VkPhysicalDevice device);

	bool HasStencilComponent(VkFormat format);

	bool CreateGraphicsPipeline(
		VulkanRenderDevice& vkDev,
		VkRenderPass renderPass, VkPipelineLayout pipelineLayout,
		const std::vector<const char*>& shaderFiles,
		VkPipeline* pipeline,
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST /* defaults to triangles*/,
		bool useDepth = true,
		bool useBlending = true,
		bool dynamicScissorState = false,
		int32_t customWidth = -1,
		int32_t customHeight = -1,
		uint32_t numPatchControlPoints = 0);

	VkResult CreateComputePipeline(VkDevice device, VkShaderModule computeShader, VkPipelineLayout pipelineLayout, VkPipeline* pipeline);

	bool CreateSharedBuffer(VulkanRenderDevice& vkDev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	bool CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	bool CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags flags = 0, uint32_t mipLevels = 1);

	bool CreateVolume(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t depth,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags flags);

	bool CreateOffscreenImage(VulkanRenderDevice& vkDev,
		VkImage& textureImage, VkDeviceMemory& textureImageMemory,
		uint32_t texWidth, uint32_t texHeight,
		VkFormat texFormat,
		uint32_t layerCount, VkImageCreateFlags flags);

	bool CreateOffscreenImageFromData(VulkanRenderDevice& vkDev,
		VkImage& textureImage, VkDeviceMemory& textureImageMemory,
		void* imageData, uint32_t texWidth, uint32_t texHeight,
		VkFormat texFormat,
		uint32_t layerCount, VkImageCreateFlags flags);

	bool CreateDepthSampler(VkDevice device, VkSampler* sampler);

	bool CreateUniformBuffer(VulkanRenderDevice& vkDev, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkDeviceSize bufferSize);

	/** Copy [data] to GPU device buffer */
	void UploadBufferData(VulkanRenderDevice& vkDev, const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, const void* data, const size_t dataSize);

	/** Copy GPU device buffer data to [outData] */
	void DownloadBufferData(VulkanRenderDevice& vkDev, const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, void* outData, size_t dataSize);

	bool CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1, uint32_t mipLevels = 1);
}












