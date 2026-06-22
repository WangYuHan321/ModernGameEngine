#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../Public/FrameGraph.h"

namespace FrameGraph
{
	// 内部资源 ID 类型（UID 区分资源种类，index + instanceId 在 ResourceID 中编码）
	using RawMemoryID = ResourceID<15>;
	using RawPipelineLayoutID = ResourceID<16>;
	using RawRenderPassID = ResourceID<17>;
	using RawFrameBufferID = ResourceID<18>;
	using RawDescriptorPoolID = ResourceID<19>;

	// 带引用计数的 ID 包装（Get / Release 管理句柄生命周期）
	using MemoryID = ResourceIDWrap<RawMemoryID>;
	using PipelineLayoutID = ResourceIDWrap<RawPipelineLayoutID>;
	using RenderPassID = ResourceIDWrap<RawRenderPassID>;
	using FrameBufferID = ResourceIDWrap<RawFrameBufferID>;
	using DescriptorPoolID = ResourceIDWrap<RawDescriptorPoolID>;

}
