#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Common.h"

namespace FrameGraph
{
	//
	//   MipmapLevel
	//

	struct MipmapLevel
	{
	private:
		uint16_t _value;

	public:

		constexpr MipmapLevel() : _value{ 0 } {}

		constexpr MipmapLevel() : _value{ 0 } {}

		explicit constexpr MipmapLevel(uint value) : _value{ value == ~0u ? uint16_t(UMax) : CheckCast<uint16_t>(value) } {}
		explicit constexpr MipmapLevel(uint64_t value) : _value{ value == UMax ? uint16_t(UMax) : CheckCast<uint16_t>(value) } {}

		GND constexpr uint	Get()								 const { return _value; }

		GND constexpr bool	operator == (const MipmapLevel& rhs) const { return _value == rhs._value; }
		GND constexpr bool	operator != (const MipmapLevel& rhs) const { return _value != rhs._value; }
		GND constexpr bool	operator >  (const MipmapLevel& rhs) const { return _value > rhs._value; }
		GND constexpr bool	operator <  (const MipmapLevel& rhs) const { return _value < rhs._value; }
		GND constexpr bool	operator >= (const MipmapLevel& rhs) const { return _value >= rhs._value; }
		GND constexpr bool	operator <= (const MipmapLevel& rhs) const { return _value <= rhs._value; }
	};

}