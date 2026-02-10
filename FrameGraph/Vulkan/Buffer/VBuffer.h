#pragma once
#include "../Public/FrameGraph.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	//
	//Vulkan 立即Buffer
	//
	class VBuffer final
	{
		friend class VBufferUnitTest;

		//类型
	private:
		using OnRelease_t = IFrameGraph::OnExternalBufferReleased_t;
		using BufferViewMap_t = HashMap<BufferViewDesc, VkBufferView>;

		//变量
	private:
		VkBuffer _buffer = VK_NULL_HANDLE;
		MemoryID _memoryId;


	};



}

