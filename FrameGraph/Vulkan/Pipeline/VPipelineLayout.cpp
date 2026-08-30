#pragma once
#include "VPipelineLayout.h"
#include "VCommon.h"
#include "../Vulkan/Instance/VDevice.h"

bool FrameGraph::VPipelineLayout::Create(const VDevice& dev, VkDescriptorSetLayout emptyLayout)
{
	EXLOCK(_drCheck);
	CHECK_ERR(_layout == VK_NULL_HANDLE);

	VPipelineLayout::VkDescriptorSetLayouts_t vk_layouts;
	VPipelineLayout::VkPushConstantRanges_t vk_pushConstants;

	for(auto& layout : vk_layouts)
	{
		layout = emptyLayout;
	}

	for(auto& pushConst : vk_pushConstants)
	{
		pushConst = {};
	}

}

void FrameGraph::VPipelineLayout::Destroy(VResourceManager&)
{

}