#pragma once


#include "StringView.h"
#include "../Common.h"
#include "../Defines.h"
#include "../Algorithms/Hash.h"

namespace FrameGraph
{
	//
	// Static String
	//
	//	栈上固定容量字符串，始终以 '\0' 结尾，可隐式转换为 StringView。
	//	常用于资源调试名（DebugName_t = StaticString<64>）。
	//

	template <typename CharT, size_t StringSize>
	struct TStaticString
	{
		STATIC_ASSERT(StringSize > 0);

		//type
	public:
		using value_type = CharT;
		using iterator = CharT*;
		using const_iterator = CharT const*;
		using View_t = BasicStringView<CharT>;
		using Self = TStaticString<CharT, StringSize>;

	private:
		CharT _array[StringSize] = {};
		size_t _length = 0;

		//method
	public:
		constexpr TStaticString() {}

		TStaticString(View_t view) { _set(view.data(), view.length()); }
		TStaticString(const CharT* str) { _set(str, _StrLen(str)); }

		template <size_t S>
		TStaticString(const CharT(&str)[S]) { _set(str, _StrLen(str)); }

		Self& operator = (View_t view) { _set(view.data(), view.length());  return *this; }
		Self& operator = (const CharT* str) { _set(str, _StrLen(str));  return *this; }

		GND constexpr size_t			size()		const { return _length; }
		GND constexpr size_t			length()	const { return _length; }
		GND constexpr bool				empty()		const { return _length == 0; }
		GND static constexpr size_t		capacity()			{ return StringSize; }

		GND CharT const*	c_str()		const { return _array; }
		GND CharT const*	data()		const { return _array; }
		GND CharT*			data()			  { return _array; }

		GND CharT&			operator [] (size_t i)			{ ASSERT(i < _length);  return _array[i]; }
		GND CharT const&	operator [] (size_t i)	const	{ ASSERT(i < _length);  return _array[i]; }

		GND iterator		begin()			{ return _array; }
		GND const_iterator	begin()	const	{ return _array; }
		GND iterator		end()			{ return _array + _length; }
		GND const_iterator	end()	const	{ return _array + _length; }

		GND bool  operator == (View_t rhs) const { return View_t{ _array, _length } == rhs; }
		GND bool  operator != (View_t rhs) const { return not (*this == rhs); }
		GND bool  operator <  (View_t rhs) const { return View_t{ _array, _length } < rhs; }
		GND bool  operator >  (View_t rhs) const { return View_t{ _array, _length } > rhs; }

		Self& operator += (View_t rhs) { _append(rhs.data(), rhs.length());  return *this; }
		Self& operator += (CharT c)
		{
			if (_length + 1 < StringSize) {
				_array[_length++] = c;
				_array[_length] = CharT{};
			}
			return *this;
		}

		void  clear() { _length = 0;  _array[0] = CharT{}; }

		GND operator View_t () const { return View_t{ _array, _length }; }

		GND HashVal  GetHash() const { return HashVal{ std::hash<View_t>{}(View_t{ _array, _length }) }; }

	private:
		GND static size_t  _StrLen(const CharT* str)
		{
			size_t n = 0;
			if (str) { while (str[n] != CharT{}) ++n; }
			return n;
		}

		void  _set(const CharT* str, size_t len)
		{
			_length = (len < StringSize) ? len : (StringSize - 1);
			for (size_t i = 0; i < _length; ++i)
				_array[i] = str[i];
			_array[_length] = CharT{};
		}

		void  _append(const CharT* str, size_t len)
		{
			const size_t avail = (StringSize - 1) - _length;
			const size_t n = (len < avail) ? len : avail;
			for (size_t i = 0; i < n; ++i)
				_array[_length + i] = str[i];
			_length += n;
			_array[_length] = CharT{};
		}
	};

	template <size_t StringSize>
	using StaticString = TStaticString< char, StringSize >;
}

namespace std
{
	template <typename CharT, size_t StringSize>
	struct hash< FrameGraph::TStaticString<CharT, StringSize> >
	{
		GND size_t  operator () (const FrameGraph::TStaticString<CharT, StringSize>& value) const
		{
			return size_t(value.GetHash());
		}
	};
}
