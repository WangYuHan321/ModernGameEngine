#pragma once

// ============================================================================
// Common.h — FrameGraph STL 公共入口
// ============================================================================
// 聚合标准库头文件、Config/Defines/TypeTraits，并定义 FrameGraph 命名空间
// 下的常用类型别名（Array、HashMap、Mutex 等）。
// 业务代码通常只需 #include "Common.h" 即可获得大部分 STL 基础设施。
// ============================================================================

#include <vector>
#include <string>
#include <functional>
#include <string_view>
#include <array>
#include <memory>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <bitset>
#include <cstring>
#include <cmath>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

#include "./Config.h"
#include "./Defines.h"
#include "./CompileTime/TypeTraits.h"

#ifndef GND
// [[nodiscard]] 简写，用于标记不应忽略返回值的函数
#if defined (_MSC_VER)
#  if _MSC_VER >= 1917
#     define  GND       [[nodiscard]]
#  endif
#elif defined(__clang__)
#     define  GND       [[nodiscard]]
#elif defined(__gcc__)
#     define  GND       [[nodiscard]]
#endif 

#ifndef GND
#   #define   GND
#endif

#endif

// 当前函数名，用于日志与断言
#ifdef _MSC_VER
#	define FUNCTION_NAME			__FUNCTION__
#elif defined(__clang__) || defined(__gcc__)
#	define FUNCTION_NAME			__func__
#else
#	define FUNCTION_NAME			"unknown function"
#endif

namespace FrameGraph
{
	// --- 基础类型别名 ---
	using uint = uint32_t;
	using String = std::string;

	// --- 容器 ---
	template <typename T>	using Array = std::vector< T >;

	template <typename T>	using UniquePtr = std::unique_ptr< T >;
	template <typename T>	using SharedPtr = std::shared_ptr< T >;
	template <typename T>	using WeakPtr = std::weak_ptr< T >;
	template <typename T> using Deque = std::deque<T>;
	template <size_t N> using BitSet = std::bitset<N>;

	// --- 同步 ---
	using Mutex = std::mutex;
	using SharedMutex = std::shared_mutex;

	// --- 函数与数组 ---
	template <typename T> using Function = std::function< T >;

	template<typename T,
			size_t ArraySize>
	using StaticArray = std::array<T, ArraySize>;

	template<typename T> using Atomic = std::atomic< T >;

	// --- 关联容器 ---
	template <typename Key,
		typename Value,
		typename Hasher = std::hash<Key>>
		using HashMap = std::unordered_map< Key, Value, Hasher >;

	template<typename FirstT,
			typename SecondT>
	using Pair = std::pair<FirstT, SecondT>;


	/*
	 * 原子操作内存顺序（Debug 下统一为 seq_cst 便于排查；Release 可定义 FRAMEGRAPH_OPTIMAL_MEMORY_ORDER 启用最优顺序）
	 *
	 *   memory_order_relaxed  — 最弱，只保证原子性
	 *   memory_order_acquire  — 读侧屏障（读锁）
	 *   memory_order_release  — 写侧屏障（写锁）
	 *   memory_order_acq_rel  — acquire + release（RMW）
	 *   memory_order_seq_cst  — 全局顺序一致（最慢）
	 */
#ifdef FRAMEGRAPH_OPTIMAL_MEMORY_ORDER

#else
	static constexpr std::memory_order	memory_order_acquire = std::memory_order_seq_cst;
	static constexpr std::memory_order	memory_order_release = std::memory_order_seq_cst;
	static constexpr std::memory_order	memory_order_acq_rel = std::memory_order_seq_cst;
	static constexpr std::memory_order	memory_order_relaxed = std::memory_order_seq_cst;
#endif


}

