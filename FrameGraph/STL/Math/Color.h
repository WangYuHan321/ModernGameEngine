#pragma once

#include "../Common.h"

namespace FrameGraph
{
	template<typename T>
	struct alignas(std::max(alignof(T), sizeof(uint))) RGBAColor
	{
		using Self = RGBAColor<T>;
		using value_type = T;


		T r, g, b, a;

		constexpr RGBAColor() : r{ T(0) }, g{ T(0) }, b{ T(0) }, a{ T(0) }
		{
			// check if supported cast from Color to array
			STATIC_ASSERT(offsetof(Self, r) + sizeof(T) == offsetof(Self, g));
			STATIC_ASSERT(offsetof(Self, g) + sizeof(T) == offsetof(Self, b));
			STATIC_ASSERT(offsetof(Self, b) + sizeof(T) == offsetof(Self, a));
			STATIC_ASSERT(sizeof(T) * (size() - 1) == (offsetof(Self, a) - offsetof(Self, r)));
		}

		constexpr RGBAColor(T r, T g, T b, T a) : r{ r }, g{ g }, b{ b }, a{ a }
		{
		}

		constexpr explicit RGBAColor(T val) : r{ val }, g{ val }, b{ val }, a{ val }
		{
		}

		template <typename B>
		constexpr explicit RGBAColor(const RGBAColor<B>& other);

		explicit RGBAColor(struct HSVColor const& hsv, T alpha = MaxValue());


		GND constexpr bool operator == (const RGBAColor<T>& rhs) const
		{
			const T  eps = Epsilon();
			return	Equals(r, rhs.r, eps) &
				Equals(g, rhs.g, eps) &
				Equals(b, rhs.b, eps) &
				Equals(a, rhs.a, eps);
		}

		GND constexpr bool operator != (const RGBAColor<T>& rhs) const { return not (*this == rhs); }

		GND static constexpr T  MaxValue()
		{
			if constexpr (IsFloatPoint<T>)
				return T(1.0);
			else
				return std::numeric_limits<T>::max();
		}

		GND static constexpr T  Epsilon()
		{
			if constexpr (IsFloatPoint<T>)
				return T(0.001);
			else
				return T(0);
		}

		GND static constexpr size_t		size() { return 4; }

		GND T* data() { return std::addressof(r); }
		GND T const* data()					const { return std::addressof(r); }

		GND T& operator [] (size_t i) { ASSERT(i < size());  return std::addressof(r)[i]; }
		GND T const& operator [] (size_t i)	const { ASSERT(i < size());  return std::addressof(r)[i]; }

	};

	using RGBA32f = RGBAColor< float >;
	using RGBA32i = RGBAColor< int >;
	using RGBA32u = RGBAColor< uint >;
	using RGBA8u = RGBAColor< uint8_t >;

}








