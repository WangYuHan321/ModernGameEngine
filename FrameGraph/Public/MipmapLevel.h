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
	GND inline MipmapLevel operator "" _mipmap(unsigned long long value)
	{
		return MipmapLevel{ CheckCast<uint64_t>(value) };
	}
}

namespace std
{
	template<>
	struct hash<FrameGraph::MipmapLevel>
	{
		size_t operator()(const FrameGraph::MipmapLevel& level) const noexcept
		{
			return size_t(FrameGraph::HashOf(level.Get()));
		}
	};
}