#pragma once

#include <string_view>

namespace FrameGraph
{
	//
	//	Fixed Size Map
	//

	template <typename Key, typename Value, size_t ArraySize>
	struct FixedMap
	{
		//types
	private:
		using Self = FixedMap<Key, Value, ArraySize>;
		using Index_t = Conditional<(ArraySize < 0xff), uint8_t, Conditional< (ArraySize < 0xFFFF), uint16_t, uint32_t >>;
		using Pair_t = Pair<const Key, Value>;

	public:
		using iterator = Pair_t*;
		using const_iterator = Pair_t const*;
		using pair_type = Pair<Key, Value>;
		using key_type = Key;
		using value_type = Value;

	private:
		mutable Index_t _indices[ArraySize];
		union {
			pair_type _array[ArraySize];
			char _buffer[sizeof(pair_type) * ArraySize];
		};
		Index_t _count = 0;

	public:
		FixedMap();
		FixedMap(Self&&);
		FixedMap(const Self&);

		~FixedMap() { clear(); }

		void clear();
	};



}

