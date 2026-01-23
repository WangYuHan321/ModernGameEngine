#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Common.h"

namespace FrameGraph
{
	
	enum class EDebugFlags : uint
	{
		LogTasks				= 1 << 0,
		LogBarriers			= 1 << 1,
		LogResourceUsage	= 1 << 2,

		VisTasks = 1 << 10,
		VisDrawTasks = 1 << 11,
		VisResources = 1 << 12,
		VisBarriers = 1 << 13,
		VisBarrierLabels = 1 << 14,
		VisTaskDependencies = 1 << 15,

		FullBarrier = 1u << 30,	// use global memory barrier addtionally to per-resource barriers
		QueueSync = 1u << 31,	// after each submit wait until queue complete execution

		Unknown = 0,

		Default = LogTasks | LogBarriers | LogResourceUsage |
		VisTasks | VisDrawTasks | VisResources | VisBarriers,

	};


}