#pragma once

#include "../Common.h"

namespace FrameGraph
{

	template <typename T, uint I>
	struct Vec;


	using bool2 = Vec< bool, 2 >;
	using bool3 = Vec< bool, 3 >;
	using bool4 = Vec< bool, 4 >;

	using short2 = Vec< int16_t, 2 >;
	using short3 = Vec< int16_t, 3 >;
	using short4 = Vec< int16_t, 4 >;

	using ushort2 = Vec< uint16_t, 2 >;
	using ushort3 = Vec< uint16_t, 3 >;
	using ushort4 = Vec< uint16_t, 4 >;

	using uint2 = Vec< uint, 2 >;
	using uint3 = Vec< uint, 3 >;
	using uint4 = Vec< uint, 4 >;

	using int2 = Vec< int, 2 >;
	using int3 = Vec< int, 3 >;
	using int4 = Vec< int, 4 >;

	using long2 = Vec< int64_t, 2 >;
	using long3 = Vec< int64_t, 3 >;
	using long4 = Vec< int64_t, 4 >;

	using float2 = Vec< float, 2 >;
	using float3 = Vec< float, 3 >;
	using float4 = Vec< float, 4 >;

	using double2 = Vec< double, 2 >;
	using double3 = Vec< double, 3 >;
	using double4 = Vec< double, 4 >;



	//
	// Vec2
	//

	template <typename T>
	struct Vec< T, 2 >
	{
		// types
		using Self = Vec<T, 2>;
		using value_type = T;

		// variables
		T	x, y;

		// methods
		constexpr Vec() : x{}, y{}
		{
			// check is supported cast Vec to array
			STATIC_ASSERT(offsetof(Self, x) + sizeof(T) == offsetof(Self, y));
			STATIC_ASSERT(sizeof(T) * (size() - 1) == (offsetof(Self, y) - offsetof(Self, x)));
		}

		constexpr Vec(T x, T y) : x{ x }, y{ y } {}

		explicit constexpr Vec(T val) : x{ val }, y{ val } {}

		template <typename B>
		constexpr Vec(const Vec<B, 2>& other) : x{ T(other.x) }, y{ T(other.y) } {}

		template <typename B>
		explicit constexpr Vec(const Vec<B, 3>& other);

		template <typename B>
		explicit constexpr Vec(const Vec<B, 4>& other);

		GND static constexpr size_t		size() { return 2; }

		GND constexpr T* data() { return std::addressof(x); }
		GND constexpr T const* data()					const { return std::addressof(x); }

		GND constexpr T& operator [] (size_t i) { ASSERT(i < size());  return std::addressof(x)[i]; }
		GND constexpr T const& operator [] (size_t i)	const { ASSERT(i < size());  return std::addressof(x)[i]; }

		GND constexpr Self		operator - ()			const { return { -x, -y }; }

		GND const Vec<T, 3>		xoy()					const;
	};



	//
	// Vec3
	//

	template <typename T>
	struct Vec< T, 3 >
	{
		// types
		using Self = Vec<T, 3>;
		using value_type = T;

		// variables
		T	x, y, z;

		// methods
		constexpr Vec() : x{}, y{}, z{}
		{
			// check is supported cast Vec to array
			STATIC_ASSERT(offsetof(Self, x) + sizeof(T) == offsetof(Self, y));
			STATIC_ASSERT(offsetof(Self, y) + sizeof(T) == offsetof(Self, z));
			STATIC_ASSERT(sizeof(T) * (size() - 1) == (offsetof(Self, z) - offsetof(Self, x)));
		}

		explicit constexpr Vec(T val) : x{ val }, y{ val }, z{ val } {}
		explicit constexpr Vec(const Vec<T, 2>& xy) : x{ xy[0] }, y{ xy[1] }, z{ T(0) } {}

		constexpr Vec(T x, T y, T z) : x{ x }, y{ y }, z{ z } {}
		constexpr Vec(const Vec<T, 2>& xy, T z) : x{ xy[0] }, y{ xy[1] }, z{ z } {}

		template <typename B>
		constexpr Vec(const Vec<B, 3>& other) : x{ T(other.x) }, y{ T(other.y) }, z{ T(other.z) } {}

		template <typename B>
		explicit constexpr Vec(const Vec<B, 4>& other);

		GND static constexpr size_t		size() { return 3; }

		GND constexpr T* data() { return std::addressof(x); }
		GND constexpr T const* data()					const { return std::addressof(x); }

		GND constexpr T& operator [] (size_t i) { ASSERT(i < size());  return std::addressof(x)[i]; }
		GND constexpr T const& operator [] (size_t i)	const { ASSERT(i < size());  return std::addressof(x)[i]; }

		GND constexpr Self		operator - ()			const { return { -x, -y, -z }; }

		GND const Vec<T, 2>		xy()					const { return { x, y }; }
		GND const Vec<T, 2>		xz()					const { return { x, z }; }
		GND const Vec<T, 3>		xzy()					const { return { x, z, y }; }
	};



	//
	// Vec4
	//

	template <typename T>
	struct Vec< T, 4 >
	{
		// types
		using Self = Vec<T, 4>;
		using value_type = T;

		// variables
		T	x, y, z, w;

		// methods
		constexpr Vec() : x{}, y{}, z{}, w{}
		{
			// check is supported cast Vec to array
			STATIC_ASSERT(offsetof(Self, x) + sizeof(T) == offsetof(Self, y));
			STATIC_ASSERT(offsetof(Self, y) + sizeof(T) == offsetof(Self, z));
			STATIC_ASSERT(offsetof(Self, z) + sizeof(T) == offsetof(Self, w));
			STATIC_ASSERT(sizeof(T) * (size() - 1) == (offsetof(Self, w) - offsetof(Self, x)));
		}

		explicit constexpr Vec(T val) : x{ val }, y{ val }, z{ val }, w{ val } {}

		constexpr Vec(T x, T y, T z, T w) : x{ x }, y{ y }, z{ z }, w{ w } {}
		constexpr Vec(const Vec<T, 3>& xyz, T w) : x{ xyz[0] }, y{ xyz[1] }, z{ xyz[2] }, w{ w } {}
		constexpr Vec(const Vec<T, 2>& xy, T z, T w) : x{ xy[0] }, y{ xy[1] }, z{ z }, w{ w } {}
		constexpr Vec(const Vec<T, 2>& xy, const Vec<T, 2>& zw) : x{ xy[0] }, y{ xy[1] }, z{ zw[0] }, w{ zw[1] } {}

		template <typename B>
		constexpr Vec(const Vec<B, 4>& other) : x{ T(other.x) }, y{ T(other.y) }, z{ T(other.z) }, w{ T(other.w) } {}

		template <typename B>
		explicit constexpr Vec(const Vec<B, 2>& other);

		template <typename B>
		explicit constexpr Vec(const Vec<B, 3>& other);

		GND static constexpr size_t		size() { return 4; }

		GND constexpr T* data() { return std::addressof(x); }
		GND constexpr T const* data()					const { return std::addressof(x); }

		GND constexpr T& operator [] (size_t i) { ASSERT(i < size());  return std::addressof(x)[i]; }
		GND constexpr T const& operator [] (size_t i)	const { ASSERT(i < size());  return std::addressof(x)[i]; }

		GND constexpr Self		operator - ()			const { return { -x, -y, -z, -w }; }

		GND const Vec<T, 2>		xy()					const { return { x, y }; }
		GND const Vec<T, 3>		xyz()					const { return { x, y, z }; }
	};



	/*
	=================================================
		constructor
	=================================================
	*/
	template <typename T>
	template <typename B>
	constexpr Vec<T, 2>::Vec(const Vec<B, 3>& other) : x{ T(other.x) }, y{ T(other.y) } {}

	template <typename T>
	template <typename B>
	constexpr Vec<T, 2>::Vec(const Vec<B, 4>& other) : x{ T(other.x) }, y{ T(other.y) } {}

	template <typename T>
	template <typename B>
	constexpr Vec<T, 3>::Vec(const Vec<B, 4>& other) : x{ T(other.x) }, y{ T(other.y) }, z{ T(other.z) } {}

	template <typename T>
	template <typename B>
	constexpr Vec<T, 4>::Vec(const Vec<B, 2>& other) : x{ T(other.x) }, y{ T(other.y) }, z{ T(0) }, w{ T(0) } {}

	template <typename T>
	template <typename B>
	constexpr Vec<T, 4>::Vec(const Vec<B, 3>& other) : x{ T(other.x) }, y{ T(other.y) }, z{ T(other.z) }, w{ T(0) } {}

	/*
	=================================================
		swizzle
	=================================================
	*/
	template <typename T>
	inline const Vec<T, 3>  Vec<T, 2>::xoy() const { return { x, T(0), y }; }

	/*
	=================================================
		operator ==
	=================================================
	*/
	template <typename T>
	GND inline constexpr bool2  operator == (const Vec<T, 2>& lhs, const Vec<T, 2>& rhs)
	{
		return { lhs.x == rhs.x, lhs.y == rhs.y };
	}

	template <typename T>
	GND inline constexpr bool3  operator == (const Vec<T, 3>& lhs, const Vec<T, 3>& rhs)
	{
		return { lhs.x == rhs.x, lhs.y == rhs.y, lhs.z == rhs.z };
	}

	template <typename T>
	GND inline constexpr bool4  operator == (const Vec<T, 4>& lhs, const Vec<T, 4>& rhs)
	{
		return { lhs.x == rhs.x, lhs.y == rhs.y, lhs.z == rhs.z, lhs.w == rhs.w };
	}

	/*
	=================================================
		operator !
	=================================================
	*/
	GND inline constexpr bool2  operator ! (const bool2& value) { return { !value.x, !value.y }; }
	GND inline constexpr bool3  operator ! (const bool3& value) { return { !value.x, !value.y, !value.z }; }
	GND inline constexpr bool4  operator ! (const bool4& value) { return { !value.x, !value.y, !value.z, !value.w }; }

	/*
	=================================================
		operator !=
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<bool, I>  operator != (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		return not (lhs == rhs);
	}

	/*
	=================================================
		operator >
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<bool, I>  operator >  (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<bool, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] > rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator >  (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs > Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator >  (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) > rhs;
	}

	/*
	=================================================
		operator <
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<bool, I>  operator < (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		return rhs > lhs;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator <  (const Vec<T, I>& lhs, const S& rhs)
	{
		return rhs > lhs;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator <  (const S& lhs, const Vec<T, I>& rhs)
	{
		return rhs > lhs;
	}

	/*
	=================================================
		operator >=
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<bool, I>  operator >= (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		return not (lhs < rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator >= (const Vec<T, I>& lhs, const S& rhs)
	{
		return not (lhs < rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator >= (const S& lhs, const Vec<T, I>& rhs)
	{
		return not (lhs < rhs);
	}

	/*
	=================================================
		operator <=
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<bool, I>  operator <= (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		return not (lhs > rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator <= (const Vec<T, I>& lhs, const S& rhs)
	{
		return not (lhs > rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<bool, I>>  operator <= (const S& lhs, const Vec<T, I>& rhs)
	{
		return not (lhs > rhs);
	}

	/*
	=================================================
		operator +=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr Vec<T, I>& operator += (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] += rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator += (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs += Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator +
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  operator + (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] + rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator + (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs + Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator + (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) + rhs;
	}

	/*
	=================================================
		operator -
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  operator - (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] - rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator - (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs - Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator - (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) - rhs;
	}

	/*
	=================================================
		operator -=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr Vec<T, I>& operator -= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] -= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator -= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs -= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator *=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr Vec<T, I>& operator *= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] *= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator *= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs *= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator *
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  operator * (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] * rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator * (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs * Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator * (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs)* rhs;
	}

	/*
	=================================================
		operator /=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr Vec<T, I>& operator /= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] /= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator /= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs /= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator /
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  operator / (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] / rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator / (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs / Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator / (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) / rhs;
	}

	/*
	=================================================
		operator %=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr Vec<T, I>& operator %= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i)
		{
			if constexpr (IsFloatPoint<T>)
				lhs[i] = std::fmod(lhs[i], rhs[i]);
			else
				lhs[i] %= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator %= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs %= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator %
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  operator % (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i)
		{
			if constexpr (IsFloatPoint<T>)
				res[i] = std::fmod(lhs[i], rhs[i]);
			else
				res[i] = lhs[i] % rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator % (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs % Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator % (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) % rhs;
	}

	/*
	=================================================
		operator <<=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr EnableIf<IsInteger<T>, Vec<T, I>&>  operator <<= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] <<= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator <<= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs <<= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator <<
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsInteger<T>, Vec<T, I>>  operator << (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] << rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator << (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs << Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator << (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) << rhs;
	}

	/*
	=================================================
		operator >>=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr EnableIf<IsInteger<T>, Vec<T, I>&>  operator >>= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] >>= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator >>= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs >>= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator >>
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsInteger<T>, Vec<T, I>>  operator >> (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] >> rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator >> (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs >> Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator >> (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) >> rhs;
	}

	/*
	=================================================
		operator &=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr EnableIf<IsInteger<T>, Vec<T, I>&>  operator &= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] &= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator &= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs &= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator &
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsInteger<T>, Vec<T, I>>  operator & (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] & rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator & (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs & Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator & (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs)& rhs;
	}

	/*
	=================================================
		operator |=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr EnableIf<IsInteger<T>, Vec<T, I>&>  operator |= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] |= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator |= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs |= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator |
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsInteger<T>, Vec<T, I>>  operator | (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] | rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator | (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs | Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator | (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) | rhs;
	}

	/*
	=================================================
		operator ^=
	=================================================
	*/
	template <typename T, uint I>
	inline constexpr EnableIf<IsInteger<T>, Vec<T, I>&>  operator ^= (Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		for (uint i = 0; i < I; ++i) {
			lhs[i] ^= rhs[i];
		}
		return lhs;
	}

	template <typename T, uint I, typename S>
	inline constexpr EnableIf<IsScalar<S>, Vec<T, I>&>  operator ^= (Vec<T, I>& lhs, const S& rhs)
	{
		return lhs ^= Vec<T, I>(rhs);
	}

	/*
	=================================================
		operator ^
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsInteger<T>, Vec<T, I>>  operator ^ (const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = lhs[i] ^ rhs[i];
		}
		return res;
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator ^ (const Vec<T, I>& lhs, const S& rhs)
	{
		return lhs ^ Vec<T, I>(rhs);
	}

	template <typename T, uint I, typename S>
	GND inline constexpr EnableIf<IsScalar<S>, Vec<T, I>>  operator ^ (const S& lhs, const Vec<T, I>& rhs)
	{
		return Vec<T, I>(lhs) ^ rhs;
	}

	/*
	=================================================
		All
	=================================================
	*/
	GND inline constexpr bool  All(const bool2& value)
	{
		return value.x & value.y;
	}

	GND inline constexpr bool  All(const bool3& value)
	{
		return value.x & value.y & value.z;
	}

	GND inline constexpr bool  All(const bool4& value)
	{
		return value.x & value.y & value.z & value.w;
	}

	/*
	=================================================
		Any
	=================================================
	*/
	GND inline constexpr bool  Any(const bool2& value)
	{
		return value.x | value.y;
	}

	GND inline constexpr bool  Any(const bool3& value)
	{
		return value.x | value.y | value.z;
	}

	GND inline constexpr bool  Any(const bool4& value)
	{
		return value.x | value.y | value.z | value.w;
	}

	/*
	=================================================
		Max
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Max(const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Max(lhs[i], rhs[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Max(const Vec<T, I>& lhs, const T& rhs)
	{
		return Max(lhs, Vec<T, I>(rhs));
	}

	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Max(const T& lhs, const Vec<T, I>& rhs)
	{
		return Max(Vec<T, I>(lhs), rhs);
	}

	/*
	=================================================
		Min
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Min(const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Min(lhs[i], rhs[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Min(const Vec<T, I>& lhs, const T& rhs)
	{
		return Min(lhs, Vec<T, I>(rhs));
	}

	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Min(const T& lhs, const Vec<T, I>& rhs)
	{
		return Min(Vec<T, I>(lhs), rhs);
	}

	/*
	=================================================
		Clamp
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Clamp(const Vec<T, I>& value, const T& minVal, const T& maxVal)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Clamp(value[i], minVal, maxVal);
		}
		return res;
	}

	/*
	=================================================
		Equals
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<bool, I>  Equals(const Vec<T, I>& lhs, const Vec<T, I>& rhs, const T& err = std::numeric_limits<T>::epsilon() * T(2))
	{
		Vec<bool, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Equals(lhs[i], rhs[i], err);
		}
		return res;
	}

	/*
	=================================================
		Floor
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsFloatPoint<T>, Vec<T, I>>  Floor(const Vec<T, I>& x)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Floor(x[i]);
		}
		return res;
	}

	/*
	=================================================
		Abs
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Abs(const Vec<T, I>& x)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Abs(x[i]);
		}
		return res;
	}

	/*
	=================================================
		Lerp
	=================================================
	*/
	template <typename T, uint I, typename B>
	GND inline constexpr Vec<T, I>  Lerp(const Vec<T, I>& a, const Vec<T, I>& b, const B& factor)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Lerp(a[i], b[i], factor);
		}
		return res;
	}

	/*
	=================================================
		Dot
	=================================================
	*/
	template <typename T>
	GND inline constexpr T  Dot(const Vec<T, 2>& lhs, const Vec<T, 2>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	template <typename T>
	GND inline constexpr T  Dot(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	template <typename T>
	GND inline constexpr T  Dot(const Vec<T, 4>& lhs, const Vec<T, 4>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	/*
	=================================================
		Cross
	=================================================
	*/
	template <typename T>
	GND inline constexpr T  Cross(const Vec<T, 2>& lhs, const Vec<T, 2>& rhs)
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	template <typename T>
	GND inline constexpr Vec<T, 3>  Cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs)
	{
		return { lhs.y * rhs.z - rhs.y * lhs.z,
				 lhs.z * rhs.x - rhs.z * lhs.x,
				 lhs.x * rhs.y - rhs.x * lhs.y };
	}

	/*
	=================================================
		Length / Length2
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsFloatPoint<T>, T>  Length2(const Vec<T, I>& value)
	{
		return Dot(value, value);
	}

	template <typename T, uint I>
	GND inline EnableIf<IsFloatPoint<T>, T>  Length(const Vec<T, I>& value)
	{
		return Sqrt(Length2(value));
	}

	/*
	=================================================
		Normalize
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr EnableIf<IsFloatPoint<T>, Vec<T, I>>  Normalize(const Vec<T, I>& value)
	{
		T  len = Dot(value, value);
		return not Equals(len, T(0)) ? (value * (T(1) / Sqrt(len))) : Vec<T, I>{ T(0) };
	}

	/*
	=================================================
		Distance / Distance2
	=================================================
	*/
	template <typename T, uint I>
	GND inline EnableIf<IsFloatPoint<T>, T>  Distance(const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		return Length(lhs - rhs);
	}

	template <typename T, uint I>
	GND inline constexpr EnableIf<IsFloatPoint<T>, T>  Distance2(const Vec<T, I>& lhs, const Vec<T, I>& rhs)
	{
		return Length2(lhs - rhs);
	}

	/*
	=================================================
		Reflect
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Reflect(const Vec<T, I>& incidentVec, const Vec<T, I>& normal)
	{
		return incidentVec - T(2) * Dot(incidentVec, normal) * normal;
	}

	/*
	=================================================
		Refract
	=================================================
	*/
	template <typename T, uint I>
	GND inline constexpr Vec<T, I>  Refract(const Vec<T, I>& incidentVec, const Vec<T, I>& normal, T eta)
	{
		T const		nd = Dot(incidentVec, normal);
		T const		k = T(1) - eta * eta * (T(1) - nd * nd);
		Vec<T, I>	r;

		if (k >= T(0))
			r = eta * incidentVec - (eta * nd + Sqrt(k)) * normal;

		return r;
	}

	/*
	=================================================
		SafeDiv
	=================================================
	*/
	template <typename T1, typename T2, typename T3, uint I>
	GND inline constexpr auto  SafeDiv(const Vec<T1, I>& lhs, const Vec<T2, I>& rhs, const T3& defVal)
	{
		using T = decltype(T1(0) + T2(0) + T3(0));

		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = SafeDiv(lhs[i], rhs[i], defVal);
		}
		return res;
	}

	template <typename T1, typename T2, uint I>
	GND inline constexpr auto  SafeDiv(const Vec<T1, I>& lhs, const Vec<T2, I>& rhs)
	{
		return SafeDiv(lhs, rhs, T1(0));
	}

	template <typename T1, typename T2, typename T3, uint I>
	GND inline constexpr auto  SafeDiv(const Vec<T1, I>& lhs, const T2& rhs, const T3& defVal)
	{
		return SafeDiv(lhs, Vec<T2, I>(rhs), defVal);
	}

	template <typename T1, typename T2, uint I>
	GND inline constexpr auto  SafeDiv(const Vec<T1, I>& lhs, const T2& rhs)
	{
		return SafeDiv(lhs, rhs, T1(0));
	}

	template <typename T1, typename T2, typename T3, uint I>
	GND inline constexpr auto  SafeDiv(const T1& lhs, const Vec<T2, I>& rhs, const T3& defVal)
	{
		return SafeDiv(Vec<T1, I>(lhs), rhs, defVal);
	}

	template <typename T1, typename T2, uint I>
	GND inline constexpr auto  SafeDiv(const T1& lhs, const Vec<T2, I>& rhs)
	{
		return SafeDiv(lhs, rhs, T1(0));
	}

	/*
	=================================================
		MinOf / MaxOf
	=================================================
	*/
	template <typename T, uint I>
	GND inline T  MinOf(const Vec<T, I>& v)
	{
		if constexpr (I == 2)
			return Min(v.x, v.y);
		if constexpr (I == 3)
			return Min(v.x, v.y, v.z);
		if constexpr (I == 4)
			return Min(v.x, v.y, v.z, v.w);
	}

	template <typename T, uint I>
	GND inline T  MaxOf(const Vec<T, I>& v)
	{
		if constexpr (I == 2)
			return Max(v.x, v.y);
		if constexpr (I == 3)
			return Max(v.x, v.y, v.z);
		if constexpr (I == 4)
			return Max(v.x, v.y, v.z, v.w);
	}

	/*
	=================================================
		Ln / Log / Log2 / Log10
	=================================================
	*/
	template <typename T, uint I>
	GND forceinline EnableIf<IsFloatPoint<T>, Vec<T, I>>  Ln(const Vec<T, I>& v)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Ln(v[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND forceinline EnableIf<IsFloatPoint<T>, Vec<T, I>>  Log2(const Vec<T, I>& v)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Log2(v[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND forceinline EnableIf<IsFloatPoint<T>, Vec<T, I>>  Log10(const Vec<T, I>& v)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Log10(v[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND forceinline EnableIf<IsFloatPoint<T>, Vec<T, I>>  Log(const Vec<T, I>& v)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Log(v[i]);
		}
		return res;
	}

	/*
	=================================================
		Wrap
	=================================================
	*/
	template <typename T, uint I>
	forceinline EnableIf<IsFloatPoint<T>, Vec<T, I>>  Wrap(const Vec<T, I>& v, const T& minValue, const T& maxValue)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Wrap(v[i], minValue, maxValue);
		}
		return res;
	}

	/*
	=================================================
		Round / RoundToInt / RoundToUint
	=================================================
	*/
	template <typename T, uint I>
	GND forceinline constexpr EnableIf<IsScalar<T> and IsFloatPoint<T>, Vec<T, I>>  Round(const Vec<T, I>& v)
	{
		Vec<T, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = Round(v[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND forceinline constexpr auto  RoundToInt(const Vec<T, I>& v)
	{
		using R = decltype(RoundToInt(T()));

		Vec<R, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = RoundToInt(v[i]);
		}
		return res;
	}

	template <typename T, uint I>
	GND forceinline constexpr auto  RoundToUint(const Vec<T, I>& v)
	{
		using R = decltype(RoundToUint(T()));

		Vec<R, I>	res;
		for (uint i = 0; i < I; ++i) {
			res[i] = RoundToUint(v[i]);
		}
		return res;
	}

}	// FrameGraph


namespace std
{
#if FG_FAST_HASH
	template <typename T, uint I>
	struct hash< FrameGraph::Vec<T, I> > {
		GND size_t  operator () (const FrameGraph::Vec<T, I>& value) const {
			return size_t(FrameGraph::HashOf(value.data(), value.size() * sizeof(T)));
		}
	};

#else
	template <typename T>
	struct hash< FrameGraph::Vec<T, 2> > {
		GND size_t  operator () (const FrameGraph::Vec<T, 2>& value) const {
			return size_t(FrameGraph::HashOf(value.x) + FrameGraph::HashOf(value.y));
		}
	};

	template <typename T>
	struct hash< FrameGraph::Vec<T, 3> > {
		GND size_t  operator () (const FrameGraph::Vec<T, 3>& value) const {
			return size_t(FrameGraph::HashOf(value.x) + FrameGraph::HashOf(value.y) + FrameGraph::HashOf(value.z));
		}
	};

	template <typename T>
	struct hash< FrameGraph::Vec<T, 4> > {
		GND size_t  operator () (const FrameGraph::Vec<T, 4>& value) const {
			return size_t(FrameGraph::HashOf(value.x) + FrameGraph::HashOf(value.y) + FrameGraph::HashOf(value.z) + FrameGraph::HashOf(value.w));
		}
	};
#endif

}








