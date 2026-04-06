#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include "../STL/Containers/Union.h"
#include "VulkanType.h"
#include "IDs.h"
#include "RenderStateEnum.h"
#include "PipelineResoure.h"

namespace FrameGraph
{
	//
	//   FrameGraph 接口
	//

	class IDrawContext
	{


		//类型
	public:
		using Context_t = Union<NullUnion, VulkanDrawContext>;

		//接口
	public:
		// 获取当前上下文

		GND virtual Context_t GetContext() = 0;

		GND virtual void Reset() = 0;

		//管线Pipeline
		virtual void BindPipeline(RawGPipelineID id, EPipelineDynamicState dynamicState = EPipelineDynamicState::Default) = 0;
		virtual void BindPipeline(RawMPipelineID id, EPipelineDynamicState dynamicState = EPipelineDynamicState::Default) = 0;

		//resource (descriptor sets)
		virtual void BindResources(const DescriptorSetID &id, const PipelineResources& resources) = 0;
		virtual void PushConstants(const PushConstantID& id, const void* data, BytesU dataSize) = 0;
		virtual void BindShadingRateImage(RawImageID value, ImageLayer layer = Default, MipmapLevel = Default) = 0;

		//vertex attributes and index buffer
		virtual void BindVertexAttributes(const VertexInputState&) = 0;
		virtual void BindIndexBuffer(const VertexBufferID& id, RawBufferID vbuf, BytesU offset) = 0;
		virtual void BindIndexBuffer(RawBufferID ibuf, BytesU offset, EIndex type) = 0;

		//render states
		virtual void SetColorBuffer(RenderTargetID id, const RenderState::ColorBuffer& value) = 0;
		virtual void SetLogicOp(ELogicOp value) = 0;
		virtual void SetBlendColor(const RGBA32f& value) = 0;
	};

}