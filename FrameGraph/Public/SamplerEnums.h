#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

namespace FrameGraph
{
	enum class EFilter
	{
		Nearest,
		Linear,

		Unknown = ~0u,
	};

	enum class EMipmapMode
	{
		Nearest,
		Linear,
		Unknown = ~0u,
	};

}