#pragma once

#include "../Config.h"
#include "../Math/Bytes.h"
#include "MemUtils.h"
#include <new>

namespace FrameGraph
{
	struct UntypedAllocator
	{
		FG_ALLOCATOR static void* Allocate(BytesU size)
		{
			return ::operator new(size_t(size), std::nothrow);
		}

		static void Deallocate(void* ptr)
		{
			::operator delete(ptr, std::nothrow);
		}

		static void Deallocate(void* ptr, BytesU size)
		{
#ifdef COMPILER_CLANG
			Unused(size);
			::operator delete(ptr, std::nothrow);
#else
			::operator delete(ptr, size_t(size));
#endif
		}

		GND bool operator == (const UntypedAllocator&) const { return true; }
	};

	struct UntypedAlignedAllocator
	{
		FG_ALLOCATOR static void* Allocate(BytesU size, BytesU align)
		{
			return ::operator new(size_t(size), std::align_val_t(size_t(align)), std::nothrow);
		}

		static void Deallocate(void* ptr, BytesU align)
		{
			::operator delete(ptr, std::align_val_t(size_t(align)), std::nothrow);
		}

		static void Deallocate(void* ptr, BytesU size, BytesU align)
		{
#ifdef COMPILER_CLANG
			Unused(size);
			::operator delete(ptr, std::align_val_t(size_t(align)), std::nothrow);
#else
			::operator delete(ptr, size_t(size), std::align_val_t(size_t(align)));
#endif
		}

		GND bool operator == (const UntypedAlignedAllocator&) const { return true; }
	};

} // FrameGraph
