#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include "./FrameGraph.h"

#include "../STL/Math/Bytes.h"
#include "../STL/Math/Vec.h"
#include "../STL/Containers/FixedArray.h"
#include "../STL/Containers/StaticString.h"
#include "./IDs.h"

namespace FrameGraph
{
	namespace Local
	{
		using TaskName_t = StaticString<64>;

		//
		//Base Draw Task
		//
		template <typename TaskType>
		struct BaseDrawTask
		{
			//variables
			TaskName_t taskName;
			RGBAu8     debugColor;

			BaseDrawTask() {};
			BaseDrawTask(StringView name, RGBAu8 color) : taskName{ name }, debugColor{ color } {}

			TaskType& SetName(StringView name) { taskName = name;  return static_cast<TaskType&>(*this); }
			TaskType& SetDebugColor(RGBA8u color) { debugColor = color;  return static_cast<TaskType&>(*this); }
		};

		//
		// Graphics Shader Debug Mode
		//
		struct GraphicsShaderDebugMode
		{
			EShaderDebugMode mode = {};
			EShaderStage stages = {};
			short2       fragCoord{ 0,0 };
		};

		//
		//Render States
		//
		struct VertexBuffer
		{
			RawBufferID id;
			BytesU    offset;
		};

		struct PushConstantData
		{
			PushConstantID id;
			Bytes<uint16_t> size;
			uint8_t data[GFG_MAXPushConstantsSize];
		};

		using PushConstants_t = FixedArray<PushConstantData, 4>;

		struct DynamicStates
		{
			EStencilOp stemcilFailOp;// 仅模板测试失败时的操作
			EStencilOp stemcilDepthOp;// 模板通过但深度测试失败时的操作 
			EStencilOp stemcilPassOp;// 模板和深度都通过时的操作
			uint8_t			stencilReference;				// front & back
			uint8_t			stencilWriteMask;				// front & back
			uint8_t			stencilCompareMask;				// front & back

			ECullMode		cullMode;

			ECompareOp		depthCompareOp;
			bool			depthTest : 1;
			bool			depthWrite : 1;
			bool			stencilTest : 1;	// enable stencil test

			bool			rasterizerDiscard : 1;
			bool			frontFaceCCW : 1;

			bool			hasStencilTest : 1;
			bool			hasStencilFailOp : 1;
			bool			hasStencilDepthFailOp : 1;
			bool			hasStencilPassOp : 1;
			bool			hasStencilReference : 1;
			bool			hasStencilWriteMask : 1;
			bool			hasStencilCompareMask : 1;
			bool			hasDepthCompareOp : 1;
			bool			hasDepthTest : 1;
			bool			hasDepthWrite : 1;
			bool			hasCullMode : 1;
			bool			hasRasterizedDiscard : 1;
			bool			hasFrontFaceCCW : 1;
			
			DynamicStates()
			{
				memset(this, 0, sizeof(*this));
			}
		};

		using ColorBuffers_t = FixedMap<Local::RenderTargetID, RenderState::ColorBuffer, 4>;
		using Scissors_t = FixedArray< RectI, GFG_MaxViewports >;

		//
		//Base Draw Call
		//
		template <typename TaskType>
		struct BaseDrawCall : BaseDrawTask<TaskType>
		{
			//types
			using StaencilValue_t = decltype(DynamicStates::stencilReference);
			using DebugMode = GraphicsShaderDebugMode;

			//variables
			PipelineResourceSet resources;
			PushConstants_t pushConstants;
			Scissors_t



		};


	}

}