#pragma once

// ============================================================================
// Config.h — FrameGraph STL 基础配置
// ============================================================================
// 编译器/平台检测、Debug/Release 开关、通用宏定义。
// 所有 STL 模块的入口配置，应在其他 STL 头文件之前（或通过 Common.h）引入。
// ============================================================================

// --- 平台检测 ---
#if defined(_WIN32) || defined(_WIN64) || defined(WIN32)
#	define PLATFORM_WINDOWS
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#	define PLATFORM_LINUX
#endif

#if defined(__ANDROID__)
#	define PLATFORM_ANDROID
#endif

// --- Debug / Release ---
#if defined(DEBUG) || defined(_DEBUG)
#	define FG_DEBUG
#else
#	define FG_RELEASE
#endif

#ifdef FG_DEBUG
#	define FG_ENABLE_DATA_RACE_CHECK
#else
#	define FG_OPTIMAL_MEMORY_ORDER
#endif

// --- 编译器检测 ---
#if defined(_MSC_VER)
#	define COMPILER_MSVC
#elif defined(__clang__)
#	define COMPILER_CLANG
#elif defined(__GNUC__)
#	define COMPILER_GCC
#endif

// --- 平台位数 ---
#if defined(_WIN64) || defined(__LP64__) || defined(__x86_64__)
#	define PLATFORM_BITS 64
#else
#	define PLATFORM_BITS 32
#endif

// --- 内存泄漏检测（MSVC Debug + FG_ENABLE_MEMLEAK_CHECKS）---
#if defined(COMPILER_MSVC) && defined(FG_ENABLE_MEMLEAK_CHECKS) && defined(FG_DEBUG)
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#	define FG_DUMP_MEMLEAKS() (::_CrtDumpMemoryLeaks() != 1)
#else
#	define FG_DUMP_MEMLEAKS() (true)
#endif

#define FG_FAST_HASH 0

// --- 参数方向注解（SAL 风格，仅作文档用途）---
#ifndef OUT
#	define OUT
#endif

#ifndef INOUT
#	define INOUT
#endif

#ifndef null
#	define null nullptr
#endif

// --- 属性与内联宏 ---
#ifndef ND_
#	if defined(_MSC_VER) && (_MSC_VER >= 1917)
#		define ND_ [[nodiscard]]
#	elif defined(__clang__) || defined(__GNUC__)
#		define ND_ [[nodiscard]]
#	else
#		define ND_
#	endif
#endif

#ifndef forceinline
#	if defined(COMPILER_MSVC)
#		define forceinline __forceinline
#	elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#		define forceinline inline __attribute__((always_inline))
#	else
#		define forceinline inline
#	endif
#endif

// MSVC 下标记自定义分配函数，便于静态分析
#ifdef COMPILER_MSVC
#	define FG_ALLOCATOR __declspec(allocator)
#else
#	define FG_ALLOCATOR
#endif

#ifndef Unused
#	define Unused(...) (void(sizeof(0), __VA_ARGS__))
#endif
