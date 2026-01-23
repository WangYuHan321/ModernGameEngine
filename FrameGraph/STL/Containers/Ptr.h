#pragma once

#include <string_view>

namespace FrameGraph
{
	//
	// Raw Pointer wrapped
	//

	template<typename T>
	struct Ptr
	{
	private:
		T* _value = null;

	public:
		Ptr() {};
		Ptr(T* ptr) : _value{ ptr } {};


		template<typename B>
		Ptr(const Ptr<B>& other) : _value{ static_cast<T*>((B*)other) } {}

		GND T* operator -> ()					const { ASSERT(_value);  return _value; }
		GND T& operator *  ()					const { ASSERT(_value);  return *_value; }
		GND T* get()							const { return _value; }

		GND explicit operator T* ()				const { return _value; }

		GND operator Ptr<const T>()				const { return _value; }

		template <typename B>
		GND explicit operator B  ()					const { return static_cast<B>(_value); }

		GND explicit operator bool()				const { return _value != null; }

		GND bool  operator == (const Ptr<T>& rhs)	const { return _value == rhs._value; }
		GND bool  operator != (const Ptr<T>& rhs)	const { return not (*this == rhs); }
	};

}

