#pragma once

// ============================================================================
// Defines.h — 断言、日志、锁宏与枚举工具
// ============================================================================
// 依赖 Config.h / Log.h，提供 ASSERT、CHECK、FG_LOGI/LOGE、EXLOCK/SHAREDLOCK、
// FG_BIT_OPERATORS、BEGIN_ENUM_CHECKS 等全局宏。
// 注意：本文件会间接引入 BitMath.h，不要在此处 include Cast.h（循环依赖）。
// ============================================================================

#include "./Config.h"
#include "./Log/Log.h"

// --- 参数方向 / 空指针 ---
#ifndef OUT
#	define OUT
#endif

#ifndef INOUT
#	define INOUT
#endif

#ifndef null
#define null nullptr
#endif

// --- 宏参数工具（用于 FG_LOGI 等可变参数宏）---
#define FG_PRIVATE_TOSTRING( ... )					#__VA_ARGS__

#define FG_PRIVATE_GETARG_0( _0_, ... ) _0_
#define FG_PRIVATE_GETARG_1( _0_, _1_, ... ) _1_
#define FG_PRIVATE_GETARG_2( _0_, _1_, _2_, ... ) _2_
#define FG_PRIVATE_UNITE( _arg0_, _arg1_ ) _arg0_ ## _arg1_
#define FG_PRIVATE_UNITE_RAW( _arg0_, _arg1_ ) FG_PRIVATE_UNITE( _arg0_, _arg1_ )

// RAII 作用域锁，配合 DataRaceCheck / RWDataRaceCheck / Mutex 使用
#ifndef EXLOCK
#	define EXLOCK( _syncObj_ )\
		std::unique_lock 	FG_PRIVATE_UNITE_RAW(__scopeLock, __COUNTER__) { _syncObj_ }
#endif

#ifndef SHAREDLOCK
#	define SHAREDLOCK( _syncObj_ ) \
		std::shared_lock	FG_PRIVATE_UNITE_RAW( __sharedLock, __COUNTER__ ) { _syncObj_ }
#endif

// --- 日志宏 ---
#ifndef FG_LOGD
#ifdef FG_DEBUG
#define FG_LOGD FG_LOGI
#else
#define FG_LOGD( ... )
#endif
#endif

#ifndef FG_LOGI
#define FG_LOGI( ... )\
		FG_PRIVATE_LOGI(FG_PRIVATE_GETARG_0( __VA_ARGS__, "" ), \
							FG_PRIVATE_GETARG_1( __VA_ARGS__, __FILE__ ), \
							FG_PRIVATE_GETARG_2( __VA_ARGS__, __FILE__, __LINE__ ))
#endif

#ifndef FG_LOGE
#define FG_LOGE( ... )\
		FG_PRIVATE_LOGE(FG_PRIVATE_GETARG_0( __VA_ARGS__, "" ), \
							FG_PRIVATE_GETARG_1( __VA_ARGS__, __FILE__ ), \
							FG_PRIVATE_GETARG_2( __VA_ARGS__, __FILE__, __LINE__ ))
#endif

#define if_likely( ... )		if ( __VA_ARGS__ )

// --- 断言与错误检查 ---
#ifndef ASSERT
# ifdef FG_DEBUG
#	define ASSERT				CHECK
# else
#	define ASSERT( ... )		{}
# endif
#endif

#ifndef CHECK
#	define FG_PRIVATE_CHECK( _expr_, _text_ ) \
		{if_likely (( _expr_ )) {} \
		 else { \
			FG_LOGE( _text_ ) \
		}}

#   define CHECK( _func_ ) \
		FG_PRIVATE_CHECK( (_func_), FG_PRIVATE_TOSTRING( _func_ ) )
#endif

#ifndef CHECK_ERR
#	define FG_PRIVATE_CHECK_ERR( _expr_, _ret_ ) \
		{if_likely (( _expr_ )) {} \
		 else { \
			FG_LOGE( FG_PRIVATE_TOSTRING ( _expr_ ) ) \
			return (_ret_); \
		}}

#   define CHECK_ERR( ... ) \
		FG_PRIVATE_CHECK_ERR( FG_PRIVATE_GETARG_0( __VA_ARGS__ ), FG_PRIVATE_GETARG_1( __VA_ARGS__, ::FrameGraph::Default ) )
#endif

#ifndef FG_PRIVATE_GETRAW
#	define FG_PRIVATE_GETRAW( _value_ ) _value_
#endif

// --- static_assert 包装 ---
#ifndef STATIC_ASSERT
#	define STATIC_ASSERT( ... ) \
		static_assert(	FG_PRIVATE_GETRAW( FG_PRIVATE_GETARG_0( __VA_ARGS__ ) ), \
						FG_PRIVATE_GETRAW( FG_PRIVATE_GETARG_1( __VA_ARGS__, FG_PRIVATE_TOSTRING(__VA_ARGS__) )))
#endif

// 为 enum class 生成位运算符（| & ~ ! 等），需配合 ToNearUInt
#define FG_BIT_OPERATORS( _type_ ) \
	GND constexpr _type_  operator |  (_type_ lhs, _type_ rhs)	{ return _type_( ToNearUInt(lhs) | ToNearUInt(rhs) ); } \
	GND constexpr _type_  operator &  (_type_ lhs, _type_ rhs)	{ return _type_( ToNearUInt(lhs) & ToNearUInt(rhs) ); } \
	\
	constexpr _type_&  operator |= (_type_ &lhs, _type_ rhs)	{ return lhs = _type_( ToNearUInt(lhs) | ToNearUInt(rhs) ); } \
	constexpr _type_&  operator &= (_type_ &lhs, _type_ rhs)	{ return lhs = _type_( ToNearUInt(lhs) & ToNearUInt(rhs) ); } \
	\
	GND constexpr _type_  operator ~ (_type_ lhs)				{ return _type_(~ToNearUInt(lhs)); } \
	GND constexpr bool   operator ! (_type_ lhs)				{ return !ToNearUInt(lhs); }

// BitMath 必须在宏定义之后；不可 include Cast.h（与 Defines 循环依赖）
#include "./Math/BitMath.h"
#include "./CompileTime/DefaultType.h"

// --- switch 枚举完整性检查（MSVC / Clang）---
#ifdef COMPILER_MSVC
#	define BEGIN_ENUM_CHECKS() \
		__pragma(warning(push)) \
		__pragma(warning(error:4061)) \
		__pragma(warning(error:4062)) \
		__pragma(warning(error:4063))
#	define END_ENUM_CHECKS() \
		__pragma(warning(pop))
#elif defined(COMPILER_CLANG)
#	define BEGIN_ENUM_CHECKS() \
		_Pragma("clang diagnostic push") \
		_Pragma("clang diagnostic error \"-Wswitch\"")
#	define END_ENUM_CHECKS() \
		_Pragma("clang diagnostic pop")
#else
#	define BEGIN_ENUM_CHECKS()
#	define END_ENUM_CHECKS()
#endif

#include <iterator>

// 数组 / 模板参数包元素个数
template <typename T>
inline constexpr size_t CountOf(T& value) { return std::size(value); }

template <typename... Types>
inline constexpr size_t CountOf() { return sizeof...(Types); }
