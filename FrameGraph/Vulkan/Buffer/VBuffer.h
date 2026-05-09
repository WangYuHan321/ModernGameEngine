#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"
#include "../Utils/VEnum.h"
#include "../VCommon.h"
#include "../STL/ThreadSafe/DataRaceCheck.h"

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
		//using BufferViewMap_t = HashMap<BufferViewDesc, VkBufferView>;

		//变量
	private:
		VkBuffer _buffer = VK_NULL_HANDLE;
		MemoryID _memoryId;
		BufferDesc _desc;

		mutable SharedMutex  _viewMapLock;
		//mutable BufferViewMap_t _viewMap;

		EQueueFamilyMask _queueFamilyMask = Default;
		VkAccessFlagBits _readAccessMask = Zero;

		DebugName_t  _debugName;
		OnRelease_t _onRelease;

		RWDataRaceCheck _drCheck;


	//方法
	public:
		VBuffer() {}
		VBuffer(VBuffer&&) = delete;
		VBuffer(const VBuffer&) = delete;




	};



}

