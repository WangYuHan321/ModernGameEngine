#pragma once

#include "../STL/Defines.h"
#include "../STL/CompileTime/TypeTraits.h"

namespace FrameGraph
{
	template <typename T>
	using NearInt = Local::Conditional<(sizeof(T) <= sizeof(int32_t)), int32_t, int64_t>;

	template <typename T>
	using NearUInt = Local::Conditional<(sizeof(T) <= sizeof(uint32_t)), uint32_t, uint64_t>;





	// NearInt<T>：根据类型T的大小，选择一个合适的整数类型（int32_t或int64_t）来表示T的近似整数值。
	template <typename T>
	inline constexpr NearInt<T> ToNearInt(T value)
	{
		return static_cast<NearInt<T>>(value);
	}

	// NearUInt<T>：根据类型T的大小，选择一个合适的整数类型（uint32_t或uint64_t）来表示T的近似整数值。
	template <typename T>
	inline constexpr NearUInt<T> ToNearUInt(T value)
	{
		return static_cast<NearUInt<T>>(value);
	}

	template <typename T>
	int  IntLog2(const T& x)
	{
		static_assert(Local::IsInteger<T> or Local::IsEnum<T>);

		constexpr int	INVALID_INDEX = std::numeric_limits<int>::min();

		unsigned long	index;

		if constexpr (sizeof(x) == 8)
			return _BitScanReverse64( & index, uint64_t(x)) ? index : INVALID_INDEX;
		
		if constexpr (sizeof(x) <= 4)
			return _BitScanReverse( & index, x) ? index : INVALID_INDEX;


	}

};








	