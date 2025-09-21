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

	//bool SetupDebugCallbacks(VkInstance instance, VkDebugUtilsMessengerEXT* messenger, VkDebugReportCallbackEXT* reportCallback);

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




}












