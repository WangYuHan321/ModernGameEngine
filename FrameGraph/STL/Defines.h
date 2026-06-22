#pragma once

#include "./Config.h"

#include "./Log/Log.h"

#ifndef OUT
#	define OUT
#endif

#ifndef INOUT
#	define INOUT
#endif

#ifndef null
#define null nullptr
#endif

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

#ifndef STATIC_ASSERT
#	define STATIC_ASSERT( ... ) \
		static_assert(	FG_PRIVATE_GETRAW( FG_PRIVATE_GETARG_0( __VA_ARGS__ ) ), \
						FG_PRIVATE_GETRAW( FG_PRIVATE_GETARG_1( __VA_ARGS__, FG_PRIVATE_TOSTRING(__VA_ARGS__) )))
#endif

// BitMath after macros; must not include Cast.h (breaks circular include with Defines)
#include "./Math/BitMath.h"
#include "./CompileTime/DefaultType.h"

#define FG_BIT_OPERATORS( _type_ ) \
	GND constexpr _type_  operator |  (_type_ lhs, _type_ rhs)	{ return _type_( ToNearUInt(lhs) | ToNearUInt(rhs) ); } \
	GND constexpr _type_  operator &  (_type_ lhs, _type_ rhs)	{ return _type_( ToNearUInt(lhs) & ToNearUInt(rhs) ); } \
	\
	constexpr _type_&  operator |= (_type_ &lhs, _type_ rhs)	{ return lhs = _type_( ToNearUInt(lhs) | ToNearUInt(rhs) ); } \
	constexpr _type_&  operator &= (_type_ &lhs, _type_ rhs)	{ return lhs = _type_( ToNearUInt(lhs) & ToNearUInt(rhs) ); } \
	\
	GND constexpr _type_  operator ~ (_type_ lhs)				{ return _type_(~ToNearUInt(lhs)); } \
	GND constexpr bool   operator ! (_type_ lhs)				{ return !ToNearUInt(lhs); }
