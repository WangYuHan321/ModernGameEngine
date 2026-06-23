#pragma once



// ============================================================================

// Cast.h — 类型转换工具

// ============================================================================

// CheckCast : 带范围/精度校验的 static_cast（Debug 下 ASSERT）

// BitCast   : 同尺寸类型的内存重解释（替代 std::bit_cast 的兼容实现）

// Cast      : void* 中间层的指针 static_cast

// ============================================================================



#include "../Config.h"

#include "../Defines.h"

#include <type_traits>

#include <cstring>



namespace FrameGraph

{



	// 安全数值转换：有符号→无符号时检查非负，并验证往返不丢失

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



	// 按位拷贝重解释，要求 sizeof 相同

	template <typename To, typename From>

	GND inline constexpr To BitCast(const From& src)

	{

		STATIC_ASSERT(sizeof(To) == sizeof(From));

		To dst{};

		std::memcpy(OUT &dst, &src, sizeof(To));

		return dst;

	}



	// 指针类型转换（经由 void*）

	template <typename R, typename T>

	GND forceinline constexpr R* Cast(T* value)

	{

		return static_cast<R*>(static_cast<void*>(value));

	}



	template <typename R, typename T>

	GND forceinline constexpr R const* Cast(T const* value)

	{

		return static_cast<R const*>(static_cast<void const*>(value));

	}



}

