#pragma once

#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include<iostream>
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

#if defined __ANDROID__
#include "VulkanAndroid.h"
#endif

#if defined(__ANDROID__)
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		LOGE("Fatal : VkResult is \" %s \" in %s at line %d", Render::Vulkan::Tool::ErrorString(res).c_str(), __FILE__, __LINE__); \
		assert(res == VK_SUCCESS);																		\
	}																									\
}
#else
#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << Render::Vulkan::Tool::ErrorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}
#endif

namespace Render
{
	namespace Vulkan
	{
		namespace Tool
		{
			void SetImageLayout(
				VkCommandBuffer commandBuffer,
				VkImage         image,
				VkImageLayout   oldImageLayout,
				VkImageLayout   newImageLayout,
				VkImageSubresourceRange subresourceRange,
				VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, //Ĭ�� ������ָ��׶ζ�Ҫ������
				VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);//Ĭ�� ������ָ��׶ζ�Ҫ������

			void SetImageLayout(
				VkCommandBuffer commandBuffer,
				VkImage         image,
				VkImageAspectFlags aspectMask,
				VkImageLayout   oldImageLayout,
				VkImageLayout   newImageLayout,
				VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, //Ĭ�� ������ָ��׶ζ�Ҫ������
				VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);//Ĭ�� ������ָ��׶ζ�Ҫ������

			void GenerateMipMap(Render::Vulkan::VulkanDevice* device,
				VkQueue queue, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

			VkBool32 GetSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat);

			VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat);

#if defined (__ANDROID__)
            VkShaderModule LoadShader(AAssetManager* assetManager, const char* fileName, VkDevice device);
#else
            VkShaderModule LoadShader(const char* fileName, VkDevice device);
#endif


			std::string ErrorString(VkResult errorCode);
		}
	}
}