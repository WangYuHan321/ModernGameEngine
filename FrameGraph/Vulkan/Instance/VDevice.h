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
	// Device Queue
	//

	struct VDeviceQueueInfo
	{
		//变量
		
		//当使用 vkQueueSubmit, vkQueueWaitIdle, vkQueueBindSparse, vkQueuePresentKHR
		mutable Mutex guard;
		VkQueue handle = VK_NULL_HANDLE;
		EQueueFamily familyIndex = Default;
		VkQueueFlags familyFlags = Default;
		float priority = 0.0f;
		uint3 minImageTransferGranularity;
		DebugName_t debugName;

		//方法
		VDeviceQueueInfo() {}
		
		VDeviceQueueInfo(VDeviceQueueInfo&& other) :
			handle{ other.handle }, familyIndex{ other.familyIndex }, familyFlags{ other.familyFlags },
			priority{ other.priority }, minImageTransferGranularity{ other.minImageTransferGranularity },
			debugName{ other.debugName }
		{
		}
	};

	//
	//  Vulkan Device
	//

	class VDevice final
	{

	};

}

