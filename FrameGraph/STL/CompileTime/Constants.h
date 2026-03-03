#pragma once

#include "../Common.h"

namespace FrameGraph
{
	struct _UMax
	{
		template<typename T>
		GND constexpr operator const T()const
		{
		}


	};

	static constexpr _UMax	UMax{};
}

