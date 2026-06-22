#pragma once

#include "ResourceEnums.h"

namespace FrameGraph
{
	//
	// Memory Description
	//

	struct MemoryDesc
	{
		EMemoryType type = EMemoryType::Default;

		MemoryDesc() {}
		explicit MemoryDesc(EMemoryType memType) : type{ memType } {}
	};

} // FrameGraph
