#pragma once

#include "ResourceEnums.h"
#include "FrameGraphEnums.h"
#include "../STL/Containers/StringView.h"

namespace FrameGraph
{
	//
	//   FrameGraph 接口
	//

	struct CommandBufferDesc
	{
		EQueueType queueType = EQueueType::Graphics;
		EDebugFlags debugFlags = EDebugFlags::Default;
		StringView name;

		CommandBufferDesc() {}
		explicit CommandBufferDesc(EQueueType type) : queueType{ type } {}

		CommandBufferDesc& SetDebugFlags(EDebugFlags value) { debugFlags = value;  return *this; }
		CommandBufferDesc& SetDebugName(StringView value) { name = value;  return *this; }
	};

	//
	// CommandBuffer interface 
	//

	class ICommandBuffer
	{
		//types
	public:
		using ExternalCmdBatch_t = union MyUnion
		{

		};


	};


}