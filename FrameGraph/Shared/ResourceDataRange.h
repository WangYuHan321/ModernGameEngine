#pragma once

#include <algorithm>

#include "../STL/Config.h"
#include "../STL/Defines.h"
#include "../STL/CompileTime/TypeTraits.h"

namespace FrameGraph
{
	//
	// Resource Data Range
	//

	template <typename T>
	struct ResourceDataRange
	{
		STATIC_ASSERT(Local::IsInteger<T>);

		using Self = ResourceDataRange<T>;
		using Value_t = T;

		T begin = T(~0ull);
		T end = T(0);

		ResourceDataRange() {}
		ResourceDataRange(T inBegin, T inEnd) : begin{ inBegin }, end{ inEnd } {}

		GND T Count() const
		{
			ASSERT(not IsEmpty());
			return end == T(~0ull) ? T(~0ull) : end - begin;
		}

		GND bool IsWhole() const { return end == T(~0ull); }
		GND bool IsEmpty() const { return end <= begin; }

		GND bool operator == (const Self& rhs) const { return begin == rhs.begin and end == rhs.end; }
		GND bool operator != (const Self& rhs) const { return not (*this == rhs); }

		GND Self Intersect(const Self& other) const
		{
			return Self{ std::max(begin, other.begin), std::min(end, other.end) };
		}

		GND bool IsIntersects(const Self& other) const
		{
			return not ((end < other.begin) or (begin > other.end));
		}

		Self& Merge(const Self& other)
		{
			ASSERT(IsIntersects(other));
			begin = std::min(begin, other.begin);
			end = std::max(end, other.end);
			return *this;
		}
	};

} // FrameGraph
