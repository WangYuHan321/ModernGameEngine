#pragma once

#include <new>
#include <utility>
#include <initializer_list>

#include "../Common.h"
#include "../Defines.h"
#include "ArrayView.h"

namespace FrameGraph
{
	//
	//	Fixed Size Array
	//
	//	栈上固定容量的数组容器：元素存放在内部对齐缓冲区中，永不重新分配，
	//	因此元素地址在容器生命周期内保持稳定（可安全存放含有 Mutex / 原子量
	//	等不可移动成员的类型，例如 VDeviceQueueInfo）。
	//

	template<typename T, size_t ArraySize>
	struct alignas(std::max(alignof(T), sizeof(void*))) FixedArray
	{
		//types
	public:
		using iterator			= T *;
		using const_iterator	= const T *;
		using value_type		= T;
		using Self				= FixedArray<T, ArraySize>;

		//variables
	private:
		size_t			_count = 0;
		alignas(T) char	_storage[sizeof(T) * ArraySize];

		//method
	public:
		FixedArray() {}

		FixedArray(std::initializer_list<T> list)
		{
			ASSERT(list.size() <= ArraySize);
			for (auto& elem : list)
				push_back(elem);
		}

		explicit FixedArray(ArrayView<T> view)
		{
			ASSERT(view.size() <= ArraySize);
			for (auto& elem : view)
				push_back(elem);
		}

		FixedArray(const Self& other)
		{
			for (size_t i = 0; i < other._count; ++i)
				push_back(other[i]);
		}

		FixedArray(Self&& other)
		{
			for (size_t i = 0; i < other._count; ++i)
				push_back(std::move(other[i]));
			other.clear();
		}

		~FixedArray() { clear(); }

		Self& operator = (const Self& rhs)
		{
			if (this == std::addressof(rhs))
				return *this;
			clear();
			for (size_t i = 0; i < rhs._count; ++i)
				push_back(rhs[i]);
			return *this;
		}

		Self& operator = (Self&& rhs)
		{
			clear();
			for (size_t i = 0; i < rhs._count; ++i)
				push_back(std::move(rhs[i]));
			rhs.clear();
			return *this;
		}

		GND T*				data()					{ return reinterpret_cast<T*>(_storage); }
		GND T const*		data()			const	{ return reinterpret_cast<T const*>(_storage); }

		GND size_t			size()			const	{ return _count; }
		GND bool			empty()			const	{ return _count == 0; }
		GND static constexpr size_t	capacity()			{ return ArraySize; }
		GND static constexpr size_t	max_size()			{ return ArraySize; }

		GND T&				operator [] (size_t i)			{ ASSERT(i < _count);  return data()[i]; }
		GND T const&		operator [] (size_t i)	const	{ ASSERT(i < _count);  return data()[i]; }

		GND iterator		begin()					{ return data(); }
		GND const_iterator	begin()			const	{ return data(); }
		GND iterator		end()					{ return data() + _count; }
		GND const_iterator	end()			const	{ return data() + _count; }

		GND T&				front()					{ ASSERT(_count > 0);  return data()[0]; }
		GND T const&		front()			const	{ ASSERT(_count > 0);  return data()[0]; }
		GND T&				back()					{ ASSERT(_count > 0);  return data()[_count - 1]; }
		GND T const&		back()			const	{ ASSERT(_count > 0);  return data()[_count - 1]; }

		void  push_back(const T& value)
		{
			ASSERT(_count < ArraySize);
			new (data() + _count) T(value);
			++_count;
		}

		void  push_back(T&& value)
		{
			ASSERT(_count < ArraySize);
			new (data() + _count) T(std::move(value));
			++_count;
		}

		template <typename ...Args>
		T&  emplace_back(Args&& ...args)
		{
			ASSERT(_count < ArraySize);
			T* ptr = new (data() + _count) T(std::forward<Args>(args)...);
			++_count;
			return *ptr;
		}

		void  pop_back()
		{
			ASSERT(_count > 0);
			--_count;
			data()[_count].~T();
		}

		void  resize(size_t newSize)
		{
			ASSERT(newSize <= ArraySize);
			while (_count > newSize)	pop_back();
			while (_count < newSize)	emplace_back();
		}

		void  clear()
		{
			for (size_t i = 0; i < _count; ++i)
				data()[i].~T();
			_count = 0;
		}

		GND bool  operator == (ArrayView<T> rhs) const { return ArrayView<T>{ data(), _count } == rhs; }
		GND bool  operator != (ArrayView<T> rhs) const { return not (*this == rhs); }

		GND operator ArrayView<T> () const { return ArrayView<T>{ data(), _count }; }
	};

}
