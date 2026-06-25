#pragma once

#include "../Memory/MemUtils.h"
#include "../Memory/UntypedAllocator.h"
#include "../Containers/StringView.h"
#include "../Algorithms/Cast.h"
#include "../Math/Bytes.h"
#include <cstring>

namespace FrameGraph
{
	// 保证非空且以 '\0' 结尾的 string_view，仅作函数参数使用
	template <typename T>
	struct NtBasicStringView
	{
	public:
		using Value_t = T;
		using Self = NtBasicStringView<T>;
		using Allocator_t = UntypedAllocator;

	private:
		static constexpr T NullChar = T(0);

		T const* _data = null;
		size_t _length = 0;
		T _buffer[32]{};
		bool _isAllocated = false;

	public:
		NtBasicStringView() : _data{ _buffer }, _length{ 0 } {}

		NtBasicStringView(Self&& other);
		NtBasicStringView(const Self& other);
		NtBasicStringView(BasicStringView<T> str);
		NtBasicStringView(const std::basic_string<T>& str);
		NtBasicStringView(const T* str);
		NtBasicStringView(const T* str, size_t length);
		~NtBasicStringView();

		Self& operator = (Self&&) = delete;
		Self& operator = (const Self&) = delete;
		Self& operator = (BasicStringView<T>) = delete;
		Self& operator = (const std::basic_string<T>&) = delete;
		Self& operator = (const T*) = delete;

		explicit operator StringView() const { return StringView{ _data, _length }; }

		GND T const* c_str() const { return _data; }
		GND size_t size() const { return _length; }
		GND size_t length() const { return _length; }
		GND bool empty() const { return _length == 0; }

	private:
		bool _Validate();
		bool _IsStatic() const { return _data == &_buffer[0]; }
	};

	using NtStringView = NtBasicStringView<char>;

	template <typename T>
	inline NtBasicStringView<T>::NtBasicStringView(BasicStringView<T> str) :
		_data{ str.data() }, _length{ str.size() }
	{
		_Validate();
	}

	template <typename T>
	inline NtBasicStringView<T>::NtBasicStringView(const std::basic_string<T>& str) :
		_data{ str.data() }, _length{ str.size() }
	{
		_Validate();
	}

	template <typename T>
	inline NtBasicStringView<T>::NtBasicStringView(const Self& other) :
		_data{ other._data }, _length{ other._length }
	{
		if (other._IsStatic())
		{
			_data = _buffer;
			std::memcpy(_buffer, other._buffer, sizeof(_buffer));
		}
		else
			_Validate();
	}

	template <typename T>
	inline NtBasicStringView<T>::NtBasicStringView(Self&& other) :
		_data{ other._data }, _length{ other._length }, _isAllocated{ other._isAllocated }
	{
		if (other._IsStatic())
		{
			_data = _buffer;
			std::memcpy(_buffer, other._buffer, sizeof(_buffer));
		}
		other._isAllocated = false;
	}

	template <typename T>
	inline NtBasicStringView<T>::NtBasicStringView(const T* str) :
		_data{ str }, _length{ str ? std::strlen(str) : 0 }
	{
		_Validate();
	}

	template <typename T>
	inline NtBasicStringView<T>::NtBasicStringView(const T* str, size_t len) :
		_data{ str }, _length{ len }
	{
		_Validate();
	}

	template <typename T>
	inline NtBasicStringView<T>::~NtBasicStringView()
	{
		if (_isAllocated)
			Allocator_t::Deallocate(const_cast<T*>(_data), BytesU::SizeOf<T>() * BytesU{ _length + 1 });
	}

	template <typename T>
	inline bool NtBasicStringView<T>::_Validate()
	{
		if (not _data)
		{
			_buffer[0] = 0;
			_data = _buffer;
			_length = 0;
			return false;
		}

		if (_data[_length] == NullChar)
			return false;

		T* new_data = null;
		const BytesU size = BytesU::SizeOf<T>() * BytesU{ _length + 1 };

		if (size > BytesU{ sizeof(_buffer) })
		{
			_isAllocated = true;
			new_data = Cast<T>(Allocator_t::Allocate(size));
		}
		else
			new_data = _buffer;

		std::memcpy(OUT new_data, _data, size_t(size));
		new_data[_length] = NullChar;
		_data = new_data;
		return true;
	}

} // FrameGraph
