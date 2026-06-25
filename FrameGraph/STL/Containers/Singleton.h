#pragma once

#include "../Memory/MemUtils.h"

namespace FrameGraph
{
	template <typename T>
	GND inline static T* Singleton()
	{
		static T inst;
		return AddressOf(inst);
	}

} // FrameGraph
