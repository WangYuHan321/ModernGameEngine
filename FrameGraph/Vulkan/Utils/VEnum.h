#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	enum class EQueueFamily : uint
	{
		_Count = 31,

		External = VK_QUEUE_FAMILY_EXTERNAL,
		Foreign = VK_QUEUE_FAMILY_FOREIGN_EXT,
		Ignored = VK_QUEUE_FAMILY_IGNORED,
		Unknown = Ignored,
	};


}

