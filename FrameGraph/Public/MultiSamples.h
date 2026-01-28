#pragma once

#include "FrameGraph.h"

namespace FrameGraph
{
	//
	//   Multi Samples
	//

	struct MultiSamples
	{
		//variables
	private:
		uint8_t _value;

		//methods
	public:
		constexpr MultiSamples() : _value(0) {}

		explicit MultiSamples(uint samples) : _value{ CheckCast<uint8_t>(IntLog2(samples)) } {}

		GND constexpr uint		Get()							const { return 1u << _value; }
		GND constexpr uint8_t	GetPowerOf2()					const { return _value; }

		GND constexpr bool		IsEnabled()					const { return _value > 0; }

		GND constexpr bool	operator == (const MultiSamples& rhs) const { return _value == rhs._value; }
		GND constexpr bool	operator != (const MultiSamples& rhs) const { return _value != rhs._value; }
		GND constexpr bool	operator >  (const MultiSamples& rhs) const { return _value > rhs._value; }
		GND constexpr bool	operator <  (const MultiSamples& rhs) const { return _value < rhs._value; }
		GND constexpr bool	operator >= (const MultiSamples& rhs) const { return _value >= rhs._value; }
		GND constexpr bool	operator <= (const MultiSamples& rhs) const { return _value <= rhs._value; }
	};

	GND inline MultiSamples operator "" _samples(unsigned long long value) { return MultiSamples{ uint(value) }; }

}