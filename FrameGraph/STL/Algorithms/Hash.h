#pragma once

#include "../Common.h"

namespace FrameGraph
{
	//
	// Hash Value
	//
	
	struct HashVal
	{
		//variables
	private:
		size_t _value = 0;

		//method
	public:
		constexpr HashVal() {}
		explicit constexpr HashVal(size_t val) : _value(val) {}

		GND constexpr bool	operator == (const HashVal& rhs)	const { return _value == rhs._value; }
		GND constexpr bool	operator != (const HashVal& rhs)	const { return not (*this == rhs); }
		GND constexpr bool	operator >  (const HashVal& rhs)	const { return _value > rhs._value; }
		GND constexpr bool  operator <  (const HashVal& rhs)	const { return _value < rhs._value; }

		constexpr HashVal& operator << (const HashVal& rhs)
		{
			const size_t	mask = (sizeof(_value) * 8 - 1);
			size_t			val = rhs._value;
			size_t			shift = 8;

			shift &= mask;
			_value ^= (val << shift) | (val >> (~(shift - 1) & mask));

			return *this;
		}

		GND constexpr const HashVal  operator + (const HashVal& rhs) const
		{
			return HashVal(*this) << rhs;
		}

		GND explicit constexpr operator size_t () const { return _value; }
	};

}

