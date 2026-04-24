#pragma once

#include "./Defines.h"

#include <vector>
#include <string>
#include <functional>    // <-- add this
#include <string_view>   // <-- add this
#include <array>
#include <memory>		// shared_ptr, weak_ptr, unique_ptr
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

#include "../STL/CompileTime/TypeTraits.h"

#ifndef GND

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

#ifdef _MSC_VER
#	define FUNCTION_NAME			__FUNCTION__
#elif defined(__clang__) || defined(__gcc__)
#	define FUNCTION_NAME			__func__
#else
#	define FUNCTION_NAME			"unknown function"
#endif

namespace FrameGraph
{
	using uint = uint32_t;
	using String = std::string;


	template <typename T>	using Array = std::vector< T >;

	template <typename T>	using UniquePtr = std::unique_ptr< T >;

	template <typename T>	using SharedPtr = std::shared_ptr< T >;
	template <typename T>	using WeakPtr = std::weak_ptr< T >;

	template <typename T> using Deque = std::deque<T>;
	template <size_t N> using BitSet = std::bitset<N>;

	using Mutex = std::mutex;
	using SharedMutex = std::shared_mutex;

	template <typename T> using Function = std::function< T >;

	template<typename T,
			size_t ArraySize>
	using StaticArray = std::array<T, ArraySize>;

	template<typename T> using Atomic = std::atomic< T >;

	template <typename Key,
		typename Value,
		typename Hasher = std::hash<Key>>
		using HashMap = std::unordered_map< Key, Value, Hasher >;

	template<typename FirstT,
			typename SecondT>
	using Pair = std::pair<FirstT, SecondT>;


	/*
	
			内存顺序	含义	性能
			memory_order_relaxed	最弱顺序，只保证原子性，不保证任何顺序约束	最快
			memory_order_acquire	后续读写不能重排到此操作之前（读锁）	较慢
			memory_order_release	之前的读写不能重排到此操作之后（写锁）	较慢
			memory_order_acq_rel	acquire + release 的组合（RMW 操作）	较慢
			memory_order_seq_cst	最强顺序，全局顺序一致性	最慢
	
	*/
#ifdef FRAMEGRAPH_OPTIMAL_MEMORY_ORDER

#else
	static constexpr std::memory_order	memory_order_acquire = std::memory_order_seq_cst;
	static constexpr std::memory_order	memory_order_release = std::memory_order_seq_cst;
	static constexpr std::memory_order	memory_order_acq_rel = std::memory_order_seq_cst;
	static constexpr std::memory_order	memory_order_relaxed = std::memory_order_seq_cst;
#endif


}

