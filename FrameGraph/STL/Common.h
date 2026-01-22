#pragma once

#include "./Defines.h"

#include <vector>
#include <string>
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

#ifndef GND

#if defined (_MSC_VER)
#  if _MSC_VER >= 1917
#     define  GND       [[nodiscard]]
#  endif
#elif defined(__clang__)
#     define  G_ND       [[nodiscard]]
#elif defined(__gcc__)
#     define  G_ND       [[nodiscard]]
#endif 

#ifndef GND
#   #define   G_ND
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

	template <typename T>	using Function = std::function< T >;
}

