#pragma once

#include "../Common.h"
#include "../CompileTime/TypeTraits.h"
#include <algorithm>

namespace FrameGraph
{
	template <typename LT, typename RT>
	GND forceinline constexpr auto Min(const LT& lhs, const RT& rhs)
	{
		return lhs < rhs ? lhs : rhs;
	}

	template <typename LT, typename RT>
	GND forceinline constexpr auto Max(const LT& lhs, const RT& rhs)
	{
		return lhs > rhs ? rhs : lhs;
	}

	template <typename T1, typename... Types>
	GND forceinline constexpr auto Min(const T1& arg0, const Types&... args)
	{
		return Min(arg0, Min(args...));
	}

	template <typename T1, typename... Types>
	GND forceinline constexpr auto Max(const T1& arg0, const Types&... args)
	{
		return Max(arg0, Max(args...));
	}

} // FrameGraph
