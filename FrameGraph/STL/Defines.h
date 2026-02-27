#pragma once

#include "./Config.h"

#ifdef  COMPILER_MSVC
#define and &&
#define or  ||
#define not !
#endif //  COMPILER_MSVC

#ifndef null
#define null nullptr
#endif // !null

#	define and		&&
#	define or		||
#	define not		!

#define FG_PRIVATE_TOSTRING( ... )					#__VA_ARGS__


//辅助宏
#define FG_PRIVATE_GETARG_0( _0_, ... ) _0_   //这里会使用_FILE_帮助
#define FG_PRIVATE_GETARG_1( _0_, _1_, ... ) _1_
#define FG_PRIVATE_GETARG_2( _0_, _1_, _2_, ... ) _2_
#define FG_PRIVATE_GETRAW( _value_ ) _value_
#define FG_PRIVATE_TOSTRING( ... ) #__VA_ARGS__
#define FG_PRIVATE_UNITE( _arg0_, _arg1_ ) _arg0_ ## _arg1_
#define FG_PRIVATE_UNITE_RAW( _arg0_, _arg1_ ) FG_PRIVATE_UNITE( _arg0_, _arg1_ )

//自动管理锁的生命周期：当程序执行超出变量作用域时，锁会自动释放   防止命名冲突：使用__COUNTER__生成唯一变量名   简化代码：一行代码就能实现加锁和解锁
//__scopeLock 作为前缀，与 __COUNTER__ 拼接后，在预处理阶段会生成一个唯一的标识符（如 __scopeLock0）。
//std::unique_lock 人为确定是互斥锁 能写

#ifndef EXLOCK
#define EXLOCK( _syncObj_ )\
		std::unique_lock 	FG_PRIVATE_UNITE_RAW(__scopeLock, __COUNTER__) { _syncObj_ }
#endif

//自动管理锁的生命周期：当程序执行超出变量作用域时，锁会自动释放   防止命名冲突：使用__COUNTER__生成唯一变量名   简化代码：一行代码就能实现加锁和解锁
//__scopeLock 作为前缀，与 __COUNTER__ 拼接后，在预处理阶段会生成一个唯一的标识符（如 __scopeLock0）。
//std::shared_lock 人为确定是共享锁 只能读 不能写
#ifndef SHAREDLOCK
#	define SHAREDLOCK( _syncObj_ ) \
		std::shared_lock	FG_PRIVATE_UNITE_RAW( __sharedLock, __COUNTER__ ) { _syncObj_ }
#endif


//log
//(text，file，line)
#ifndef FG_LOGD
#ifdef FG_DEBUG
#define FG_LOGD FG_LOGI
#else
#define FG_LOGD( ... )
#endif
#endif // FG_DEBUG

#ifndef FG_LOGI
#define FG_LOGI( ... )\
		FG_PRIVATE_LOGI(FG_PRIVATE_GETARG_0( __VA_ARGS__, "" ), \
							FG_PRIVATE_GETARG_1( __VA_ARGS__, __FILE__ ), \
							FG_PRIVATE_GETARG_2( __VA_ARGS__, __FILE__, __LINE__ ))
#endif




// debug only check
#ifndef ASSERT
# ifdef FG_DEBUG
#	define ASSERT				CHECK
# else
#	define ASSERT( ... )		{}
# endif
#endif

// check function return value
#ifndef CHECK
#	define FG_PRIVATE_CHECK( _expr_, _text_ ) \
		{if_likely (( _expr_ )) {} \
		 else { \
			FG_LOGE( _text_ ); \
		}}

#   define CHECK( _func_ ) \
		FG_PRIVATE_CHECK( (_func_), FG_PRIVATE_TOSTRING( _func_ ) )
#endif


#ifndef forceinline

#define forceinline inline

#endif


#ifdef COMPILER_MSVC


#endif








