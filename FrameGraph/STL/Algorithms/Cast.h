#pragma once

#include "../Config.h"
#include "../Defines.h"
#include <type_traits>

namespace FrameGraph
{

	template <typename To, typename From>
	GND forceinline constexpr To CheckCast(const From& src)
	{
		if constexpr (std::is_signed_v<From> and std::is_unsigned_v<To>)
		{
			ASSERT(src >= 0);
		}

		ASSERT(static_cast<From>(static_cast<To>(src)) == src);

		return static_cast<To>(src);
	}

}
