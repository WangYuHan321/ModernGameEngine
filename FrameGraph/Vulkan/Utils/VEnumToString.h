#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	GND inline String VkPipelineStage_ToString(const VkPipelineStageFlags values)
	{
		String result;
		if (values & VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
			result += "TopOfPipe|";
		if (values & VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT)
			result += "DrawIndirect|";
		if (values & VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)
			result += "VertexInput|";
		if (values & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT)
			result += "VertexShader|";
		if (values & VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT)
			result += "TessellationControlShader|";
		if (values & VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT)
			result += "TessellationEvaluationShader|";
		if (values & VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)
			result += "GeometryShader|";
		if (values & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			result += "FragmentShader|";
		if (values & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)
			result += "EarlyFragmentTests|";
		if (values & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)
			result += "LateFragmentTests|";
		if (values & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			result += "ColorAttachmentOutput|";
		if (values & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
			result += "ComputeShader|";
		if (values & VK_PIPELINE_STAGE_TRANSFER_BIT)
			result += "Transfer|";
		if (values & VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
			result += "BottomOfPipe|";
		if (values & VK_PIPELINE_STAGE_HOST_BIT)
			result += "Host|";
		if (values & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT)
			result += "AllGraphics|";
		if (values & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
			result += "AllCommands|";
		return result.empty() ? "Unknown" : result.substr(0, result.size() - 1);
	}



}

