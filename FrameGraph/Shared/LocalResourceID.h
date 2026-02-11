#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../Public/FrameGraph.h"

namespace FrameGraph
{
	using RawMemoryID = FrameGraph::Local::ResourceID<15>;
	using RawPipelineLayoutID = FrameGraph::Local::ResourceID<16>;
	using RawRenderPassID = FrameGraph::Local::ResourceID<17>;
	using RawFrameBufferID = FrameGraph::Local::ResourceID<18>;
	using RawDescriptorPoolID = FrameGraph::Local::ResourceID<19>;

	using MemoryID = FrameGraph::Local::ResourceIDWrap<RawMemoryID>;
	using PipelineLayoutID = FrameGraph::Local::ResourceIDWrap<RawPipelineLayoutID>;
	using RenderPassID = FrameGraph::Local::ResourceIDWrap<RawRenderPassID>;
	using FrameBufferID = FrameGraph::Local::ResourceIDWrap<RawFrameBufferID>;
	using DescriptorPoolID = FrameGraph::Local::ResourceIDWrap<RawDescriptorPoolID>;

}