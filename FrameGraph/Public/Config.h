#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include <chrono>
#include <atomic>

#include "../STL/Common.h"

namespace FrameGraph
{
#ifdef PLATOFRM_ANDROID
	// buffer
	static constexpr unsigned	GFG_MaxVertexBuffers = 8;
	static constexpr unsigned	GFG_MaxVertexAttribs = 16;

	// render pass
	static constexpr unsigned	GFG_MaxColorBuffers = 4;
	static constexpr unsigned	GFG_MaxViewports = 1;

	// pipeline
	static constexpr bool		GFG_EnableShaderDebugging = false;
	static constexpr unsigned	GFG_MaxDescriptorSets = 4;
	static constexpr unsigned	GFG_MaxBufferDynamicOffsets = 12;	// 8 UBO + 4 SSBO
	static constexpr unsigned	GFG_MaxElementsInUnsizedDesc = 1;	// if used extension GL_EXT_nonuniform_qualifier

	// memory
	static constexpr unsigned	GFG_VkDevicePageSizeMb = 64;
#else
	//Buffer
	static constexpr unsigned GFG_MaxVertexBuffers = 8;
	static constexpr unsigned GFG_MaxVertexAttribs = 16;

	//Render Pass
	static constexpr unsigned GFG_MaxColorBuffers = 8;
	static constexpr unsigned GFG_MaxViewports = 16;

	//pipeline
	static constexpr bool	  GFG_EnableShaderDebugging = true;
	static constexpr unsigned GFG_MaxDescriptorSets = 8;
	static constexpr unsigned GFG_MaxBufferDynamicOffsets = 16;
	static constexpr unsigned GFG_MaxElementsInUnsizedDesc = 64;

	//memory
	static constexpr unsigned GFG_VkDevicePaaaageSizeMb = 256;

#endif

	//render pass
	static constexpr unsigned GFG_MaxRenderPassUbpasses = 8;

	// pipeline
	static constexpr unsigned	GFG_MaxPushConstants = 8;
	static constexpr unsigned	GFG_MaxPushConstantsSize = 128;	// bytes
	static constexpr unsigned	GFG_MaxSpecConstants = 8;
	static constexpr unsigned	GFG_DebugDescriptorSet = GFG_MaxDescriptorSets - 1;

	// queue
	static constexpr unsigned	GFG_MaxQueueFamilies = 32;

	// task
	static constexpr unsigned	GFG_MaxTaskDependencies = 8;
	static constexpr unsigned	GFG_MaxCopyRegions = 8;
	static constexpr unsigned	GFG_MaxClearRanges = 8;
	static constexpr unsigned	GFG_MaxBlitRegions = 8;
	static constexpr unsigned	GFG_MaxResolveRegions = 8;
	static constexpr unsigned	GFG_MaxDrawCommands = 4;

}