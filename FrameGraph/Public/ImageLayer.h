#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Common.h"

namespace FrameGraph
{
	//
	// Image Array Layer
	//

	struct ImageLayer
	{
		// variables
	private:
		uint		_value = 0;


		// methods
	public:
		constexpr ImageLayer() {}
		explicit constexpr ImageLayer(uint value) : _value{ value } {}
		explicit constexpr ImageLayer(uint64_t value) : _value{ CheckCast<uint>(value) } {}

		GND constexpr uint	Get()								const { return _value; }

		GND ImageLayer		operator + (const ImageLayer& rhs)	const { return ImageLayer{ Get() + rhs.Get() }; }

		GND constexpr bool	operator == (const ImageLayer& rhs) const { return _value == rhs._value; }
		GND constexpr bool	operator != (const ImageLayer& rhs) const { return _value != rhs._value; }
		GND constexpr bool	operator >  (const ImageLayer& rhs) const { return _value > rhs._value; }
		GND constexpr bool	operator <  (const ImageLayer& rhs) const { return _value < rhs._value; }
		GND constexpr bool	operator >= (const ImageLayer& rhs) const { return _value >= rhs._value; }
		GND constexpr bool	operator <= (const ImageLayer& rhs) const { return _value <= rhs._value; }
	};

	GND inline ImageLayer operator "" _layer(unsigned long long value)
	{
		return ImageLayer{ CheckCast<uint64_t>(value) };
	}
}

namespace std
{
	template<>
	struct hash<FrameGraph::ImageLayer>
	{
		size_t operator()(const FrameGraph::ImageLayer& layer) const noexcept
		{
			return size_t(FrameGraph::HashOf(layer.Get()));
		}
	};
}