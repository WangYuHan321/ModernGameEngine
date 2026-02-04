#pragma once

#include "../Common.h"
#include "../STL/Algorithms/Cast.h"

namespace FrameGraph
{

	template <typename T>
	struct Rectangle
	{
		// types
		using Vec2_t = Vec<T, 2>;
		using Self = Rectangle<T>;


		// variables
		T	left, top;
		T	right, bottom;


		// methods
		constexpr Rectangle() :
			left{ T(0) }, top{ T(0) }, right{ T(0) }, bottom{ T(0) }
		{
			// check is supported cast Rectangle to array
			STATIC_ASSERT(offsetof(Self, left) + sizeof(T) == offsetof(Self, top));
			STATIC_ASSERT(offsetof(Self, top) + sizeof(T) == offsetof(Self, right));
			STATIC_ASSERT(offsetof(Self, right) + sizeof(T) == offsetof(Self, bottom));
			STATIC_ASSERT(sizeof(T[3]) == (offsetof(Self, bottom) - offsetof(Self, left)));
		}

		constexpr Rectangle(T left, T top, T right, T bottom) :
			left{ left }, top{ top }, right{ right }, bottom{ bottom } {
		}

		constexpr Rectangle(const Vec2_t& leftTop, const Vec2_t& rightBottom) :
			left{ leftTop.x }, top{ leftTop.y }, right{ rightBottom.x }, bottom{ rightBottom.y } {
		}

		constexpr explicit Rectangle(const Vec2_t& size) :
			Rectangle{ Vec2_t{}, size } {
		}

		constexpr Rectangle(const Self& other) :
			left{ other.left }, top{ other.top }, right{ other.right }, bottom{ other.bottom } {
		}

		template <typename B>
		constexpr explicit Rectangle(const Rectangle<B>& other) :
			left{ T(other.left) }, top{ T(other.top) }, right{ T(other.right) }, bottom{ T(other.bottom) } {
		}

		Self& LeftTop(const Vec2_t& v);
		Self& RightBottom(const Vec2_t& v);

		GND constexpr const T		Width()		const { return right - left; }
		GND constexpr const T		Height()		const { return bottom - top; }
		GND constexpr const T		CenterX()		const { return (right + left) / T(2); }
		GND constexpr const T		CenterY()		const { return (top + bottom) / T(2); }

		GND constexpr const Vec2_t	LeftTop()		const { return { left, top }; }
		GND constexpr const Vec2_t	RightBottom()	const { return { right, bottom }; }
		GND constexpr const Vec2_t	LeftBottom()	const { return { left, bottom }; }
		GND constexpr const Vec2_t	RightTop()		const { return { right, top }; }

		GND T const* data()			const { return std::addressof(left); }
		GND T* data() { return std::addressof(left); }

		GND constexpr const Vec2_t	Size()			const { return { Width(), Height() }; }
		GND constexpr const Vec2_t	Center()		const { return { CenterX(), CenterY() }; }

		GND constexpr bool			IsEmpty()		const { return Equals(left, right) | Equals(top, bottom); }
		GND constexpr bool			IsInvalid()	const { return (right < left) | (bottom < top); }
		GND constexpr bool			IsValid()		const { return (not IsEmpty()) & (not IsInvalid()); }

		GND constexpr bool  IsNormalized() const;
		Self& Normalize();

		GND constexpr bool	Intersects(const Vec2_t& point) const;
		GND constexpr bool	Intersects(const Self& point) const;

		GND constexpr Self	Intersection(const Self& other) const;

		GND constexpr bool4 operator == (const Self& rhs) const;

		Self& Join(const Self& other);
		Self& Join(const Vec2_t& point);

		Self& Stretch(const Self& size);
		Self& Stretch(const Vec2_t& size);
		Self& Stretch(T size);
	};


	using RectU = Rectangle< uint >;
	using RectI = Rectangle< int >;
	using RectF = Rectangle< float >;

}








