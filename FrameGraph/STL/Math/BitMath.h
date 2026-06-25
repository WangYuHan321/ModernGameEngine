#pragma once

#include "../Config.h"
#include "../CompileTime/TypeTraits.h"
#include <limits>

#ifdef COMPILER_MSVC
# include <intrin.h>
# pragma intrinsic(_BitScanForward, _BitScanReverse)
# if PLATFORM_BITS == 64
#  pragma intrinsic(_BitScanForward64, _BitScanReverse64)
# endif
#endif

namespace FrameGraph
{
	template <typename T>
	using NearInt = Local::Conditional<(sizeof(T) <= sizeof(int32_t)), int32_t, int64_t>;

	template <typename T>
	using NearUInt = Local::Conditional<(sizeof(T) <= sizeof(uint32_t)), uint32_t, uint64_t>;

	template <typename T>
	inline constexpr NearInt<T> ToNearInt(T value)
	{
		return static_cast<NearInt<T>>(value);
	}

	template <typename T>
	inline constexpr NearUInt<T> ToNearUInt(T value)
	{
		return static_cast<NearUInt<T>>(value);
	}

	template <typename T1, typename T2>
	inline constexpr bool AllBits(const T1& lhs, const T2& rhs)
	{
		return (ToNearUInt(lhs) & ToNearUInt(rhs)) == ToNearUInt(rhs);
	}

	template <typename T1, typename T2>
	inline constexpr bool AnyBits(const T1& lhs, const T2& rhs)
	{
		return (ToNearUInt(lhs) & ToNearUInt(rhs)) != 0;
	}

	template <typename T>
	inline constexpr bool IsPowerOfTwo(const T& x)
	{
		static_assert(Local::IsEnum<T> or Local::IsInteger<T>);

		using U = NearUInt<T>;
		const U val = U(x);
		return (val != 0) and ((val & (val - U(1))) == U(0));
	}

	template <typename T>
	inline int IntLog2(const T& x)
	{
		static_assert(Local::IsInteger<T> or Local::IsEnum<T>);

		constexpr int INVALID_INDEX = std::numeric_limits<int>::min();

#ifdef COMPILER_MSVC
		unsigned long index = 0;

# if PLATFORM_BITS == 64
		if constexpr (sizeof(x) == 8)
			return _BitScanReverse64(&index, uint64_t(x)) ? int(index) : INVALID_INDEX;
# endif
		if constexpr (sizeof(x) <= 4)
			return _BitScanReverse(&index, static_cast<unsigned long>(x)) ? int(index) : INVALID_INDEX;

		return INVALID_INDEX;
#else
		if constexpr (sizeof(x) == 8)
			return x ? int((sizeof(x) * 8) - 1 - __builtin_clzll(uint64_t(x))) : INVALID_INDEX;
		else
			return x ? int((sizeof(x) * 8) - 1 - __builtin_clz(uint(x))) : INVALID_INDEX;
#endif
	}

} // FrameGraph
