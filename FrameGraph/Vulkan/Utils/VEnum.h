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

	enum class ExeOrderIndex : uint
	{
		Initial = 0,
		First = 1,
		Final = 0x80000000,
		Unknown = ~0u,
	};

	enum class EQueueFamilyMask : uint
	{
		All = ~0u,
		Unknown = 0,
	};
	
	FG_BIT_OPERATORS(EQueueFamilyMask);


}

