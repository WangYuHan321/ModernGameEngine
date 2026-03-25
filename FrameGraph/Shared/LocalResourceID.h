#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../Public/FrameGraph.h"

namespace FrameGraph
{
	using RawMemoryID = ResourceID<15>;
	using RawPipelineLayoutID = ResourceID<16>;
	using RawRenderPassID = ResourceID<17>;
	using RawFrameBufferID = ResourceID<18>;
	using RawDescriptorPoolID = ResourceID<19>;

	using MemoryID = ResourceIDWrap<RawMemoryID>;
	using PipelineLayoutID = ResourceIDWrap<RawPipelineLayoutID>;
	using RenderPassID = ResourceIDWrap<RawRenderPassID>;
	using FrameBufferID = ResourceIDWrap<RawFrameBufferID>;
	using DescriptorPoolID = ResourceIDWrap<RawDescriptorPoolID>;

}