#pragma once

#include "../Common.h"
#include <atomic>

namespace FrameGraph
{
	// 带 acquire/release 语义的原子指针（ChunkedIndexedPool 写入 value chunk 时使用）
	template <typename T>
	struct AtomicPtr
	{
	private:
		Atomic<T*> _ptr{ null };

	public:
		AtomicPtr() = default;
		explicit AtomicPtr(T* ptr) : _ptr{ ptr } {}

		void Store(T* ptr) { _ptr.store(ptr, std::memory_order_release); }
		GND T* Load() const { return _ptr.load(std::memory_order_acquire); }

		AtomicPtr& operator = (T* ptr) { Store(ptr); return *this; }
		GND explicit operator T* () const { return Load(); }
		GND bool operator == (const T* rhs) const { return Load() == rhs; }
		GND bool operator != (const T* rhs) const { return not (*this == rhs); }
	};

	// 非原子指针包装，接口与 AtomicPtr 一致
	template <typename T>
	struct NonAtomicPtr
	{
	private:
		T* _ptr = null;

	public:
		NonAtomicPtr() = default;
		explicit NonAtomicPtr(T* ptr) : _ptr{ ptr } {}

		void Store(T* ptr) { _ptr = ptr; }
		GND T* Load() const { return _ptr; }

		NonAtomicPtr& operator = (T* ptr) { Store(ptr); return *this; }
		GND explicit operator T* () const { return Load(); }
		GND bool operator == (const T* rhs) const { return Load() == rhs; }
		GND bool operator != (const T* rhs) const { return not (*this == rhs); }
	};

} // FrameGraph
