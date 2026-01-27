#pragma once

#include "./Config.h"
#include "../Common.h"

namespace FrameGraph
{
	template <typename To, typename From>
	GND inline constexpr To  CheckCast(const From& src)
	{
		if constexpr (std::is_signed_v<From> and std::is_unsigned_v<To>)
		{
			ASSERT(src >= 0);
		}

		ASSERT(static_cast<From>(static_cast<To>(src)) == src);

		return static_cast<To>(src);
	}

}










