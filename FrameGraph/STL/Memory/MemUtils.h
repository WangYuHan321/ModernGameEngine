#pragma once

#include "../Math/Bytes.h"
#include "../CompileTime/TypeTraits.h"
#include <new>

namespace FrameGraph
{
	template <typename T>
	GND forceinline decltype(auto) AddressOf(T& value)
	{
		return std::addressof(value);
	}

	template <typename T, typename... Types>
	forceinline T* PlacementNew(OUT void* ptr, Types&&... args)
	{
		return new (ptr) T{ std::forward<Types>(args)... };
	}

	forceinline void MemCopy(void* dst, BytesU dstSize, const void* src, BytesU srcSize)
	{
		ASSERT(srcSize <= dstSize);
		ASSERT(dst and src);
		std::memcpy(dst, src, size_t(std::min(srcSize, dstSize)));
	}

} // FrameGraph
