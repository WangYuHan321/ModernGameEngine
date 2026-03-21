#pragma once

#include "../Common.h"
#include "../Defines.h"

namespace FrameGraph
{
	struct _UMax
	{
		template<typename T>
		GND constexpr operator const T()const
		{
			STATIC_ASSERT(T(~T(0)) > T(0));  // 确保是无符号类型
			return T(~T(0));                    // 返回所有位都为1的值
		}

		template <typename T>
		GND friend constexpr bool operator == (const T& left, const _UMax& right)
		{
			return T(right) == left;
		}

		template <typename T>
		GND friend constexpr bool operator != (const T& left, const _UMax& right)
		{
			return T(right) != left;
		}

	};

	static constexpr _UMax	UMax{};

	struct _Zero
	{
		template<typename T>
		GND constexpr operator const T()const
		{
			////STATIC_ASSERT( std::is_integral_v<T> || std::is_enum_v<T> );
			return T(0);
		}

		template <typename T>
		GND friend constexpr auto  operator == (const T& left, const _Zero& right)
		{
			return T(right) == left;
		}

		template <typename T>
		GND friend constexpr auto  operator != (const T& left, const _Zero& right)
		{
			return T(right) != left;
		}

		template <typename T>
		GND friend constexpr auto  operator > (const T& left, const _Zero& right)
		{
			return left > T(right);
		}

		template <typename T>
		GND friend constexpr auto  operator < (const T& left, const _Zero& right)
		{
			return left < T(right);
		}

		template <typename T>
		GND friend constexpr auto  operator >= (const T& left, const _Zero& right)
		{
			return left >= T(right);
		}

		template <typename T>
		GND friend constexpr auto  operator <= (const T& left, const _Zero& right)
		{
			return left <= T(right);
		}
	};

	static constexpr _Zero	Zero{};
}

