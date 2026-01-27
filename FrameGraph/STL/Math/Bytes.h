#pragma once

#include "../Common.h"
#include "../STL/Algorithms/Cast.h"

namespace FrameGraph
{
	//
	//Bytes
	//

	template <typename T>
	struct Bytes
	{
		static_assert(IsInteger<T> and IsScalar<T>, "must be integer scalar");

		//types
	public:
		using Value_t = T;

		//variables
	private:
		T _value;

		//method
	public:
		constexpr Bytes() : _value(0) {}

		explicit constexpr Bytes(T value) : _value(value) {}

		template <typename B>
		explicit constexpr Bytes(const Bytes<B>& other) : _value(T(other)) {}

		GND explicit constexpr operator int8_t ()	const { return CheckCast<int8_t>(_value); }
		GND explicit constexpr operator int16_t ()	const { return CheckCast<int16_t>(_value); }
		GND explicit constexpr operator int()		const { return CheckCast<int>(_value); }
		GND explicit constexpr operator int64_t ()	const { return CheckCast<int64_t>(_value); }

		GND explicit constexpr operator uint8_t ()	const { return CheckCast<uint8_t>(_value); }
		GND explicit constexpr operator uint16_t ()	const { return CheckCast<uint16_t>(_value); }
		GND explicit constexpr operator uint32_t ()	const { return CheckCast<uint32_t>(_value); }
		GND explicit constexpr operator uint64_t ()	const { return CheckCast<uint64_t>(_value); }

		template <typename R>
		GND explicit constexpr operator R* ()		const { return BitCast<R*>(CheckCast<size_t>(_value)); }

		GND constexpr T		Kb()	const { return _value >> 10; }
		GND constexpr T		Mb()	const { return _value >> 20; }
		GND constexpr T		Gb()	const { return _value >> 30; }

		GND static constexpr Bytes<T>	FromBits(T value) { return Bytes<T>(value >> 3); }
		GND static constexpr Bytes<T>	FromKb(T value) { return Bytes<T>(value << 10); }
		GND static constexpr Bytes<T>	FromMb(T value) { return Bytes<T>(value << 20); }
		GND static constexpr Bytes<T>	FromGb(T value) { return Bytes<T>(value << 30); }


		template <typename B>	GND static constexpr Bytes<T>	SizeOf() { return Bytes<T>(sizeof(B)); }
		template <typename B>	GND static constexpr Bytes<T>	SizeOf(const B&) { return Bytes<T>(sizeof(B)); }

		template <typename B>	GND static constexpr Bytes<T>	AlignOf() { return Bytes<T>(alignof(B)); }
		template <typename B>	GND static constexpr Bytes<T>	AlignOf(const B&) { return Bytes<T>(alignof(B)); }


		// move any pointer
		template <typename B>	GND friend B* operator +  (B* lhs, const Bytes<T>& rhs) { return BitCast<B*>(size_t(lhs) + size_t(rhs._value)); }
		template <typename B>	GND friend B* operator -  (B* lhs, const Bytes<T>& rhs) { return BitCast<B*>(size_t(lhs) - size_t(rhs._value)); }
		template <typename B>		friend B*& operator += (B*& lhs, const Bytes<T>& rhs) { return (lhs = lhs + rhs); }
		template <typename B>		friend B*& operator -= (B*& lhs, const Bytes<T>& rhs) { return (lhs = lhs + rhs); }


		GND constexpr Bytes<T>	operator ~ () const { return Bytes<T>(~_value); }

		Bytes<T>& operator += (const Bytes<T>& rhs) { _value += rhs._value;  return *this; }
		GND constexpr Bytes<T>  operator +  (const Bytes<T>& rhs) const { return Bytes<T>(_value + rhs._value); }

		Bytes<T>& operator -= (const Bytes<T>& rhs) { _value -= rhs._value;  return *this; }
		GND constexpr Bytes<T>  operator -  (const Bytes<T>& rhs) const { return Bytes<T>(_value - rhs._value); }

		Bytes<T>& operator *= (const Bytes<T>& rhs) { _value *= rhs._value;  return *this; }
		GND constexpr Bytes<T>  operator *  (const Bytes<T>& rhs) const { return Bytes<T>(_value * rhs._value); }

		Bytes<T>& operator /= (const Bytes<T>& rhs) { _value /= rhs._value;  return *this; }
		GND constexpr Bytes<T>  operator /  (const Bytes<T>& rhs) const { return Bytes<T>(_value / rhs._value); }

		Bytes<T>& operator %= (const Bytes<T>& rhs) { _value %= rhs._value;  return *this; }
		GND constexpr Bytes<T>  operator %  (const Bytes<T>& rhs) const { return Bytes<T>(_value % rhs._value); }


		Bytes<T>& operator += (const T rhs) { _value += rhs;  return *this; }
		GND constexpr Bytes<T>  operator +  (const T rhs) const { return Bytes<T>(_value + rhs); }
		GND friend Bytes<T>		operator +  (T lhs, const Bytes<T>& rhs) { return Bytes<T>(lhs + rhs._value); }

		Bytes<T>& operator -= (const T rhs) { _value -= rhs;  return *this; }
		GND constexpr Bytes<T>  operator -  (const T rhs) const { return Bytes<T>(_value - rhs); }
		GND friend Bytes<T>		operator -  (T lhs, const Bytes<T>& rhs) { return Bytes<T>(lhs - rhs._value); }

		Bytes<T>& operator *= (const T rhs) { _value *= rhs;  return *this; }
		GND constexpr Bytes<T>  operator *  (const T rhs) const { return Bytes<T>(_value * rhs); }
		GND friend Bytes<T>		operator *  (T lhs, const Bytes<T>& rhs) { return Bytes<T>(lhs * rhs._value); }

		Bytes<T>& operator /= (const T rhs) { _value /= rhs;  return *this; }
		GND constexpr Bytes<T>  operator /  (const T rhs) const { return Bytes<T>(_value / rhs); }
		GND friend Bytes<T>		operator /  (T lhs, const Bytes<T>& rhs) { return Bytes<T>(lhs / rhs._value); }

		Bytes<T>& operator %= (const T rhs) { _value %= rhs;  return *this; }
		GND constexpr Bytes<T>  operator %  (const T rhs) const { return Bytes<T>(_value % rhs); }
		GND friend Bytes<T>		operator %  (T lhs, const Bytes<T>& rhs) { return Bytes<T>(lhs % rhs._value); }


		GND constexpr bool		operator == (const Bytes<T>& rhs) const { return _value == rhs._value; }
		GND constexpr bool		operator != (const Bytes<T>& rhs) const { return _value != rhs._value; }
		GND constexpr bool		operator >  (const Bytes<T>& rhs) const { return _value > rhs._value; }
		GND constexpr bool		operator <  (const Bytes<T>& rhs) const { return _value < rhs._value; }
		GND constexpr bool		operator >= (const Bytes<T>& rhs) const { return _value >= rhs._value; }
		GND constexpr bool		operator <= (const Bytes<T>& rhs) const { return _value <= rhs._value; }

		GND constexpr bool		operator == (const T rhs) const { return _value == rhs; }
		GND constexpr bool		operator != (const T rhs) const { return _value != rhs; }
		GND constexpr bool		operator >  (const T rhs) const { return _value > rhs; }
		GND constexpr bool		operator <  (const T rhs) const { return _value < rhs; }
		GND constexpr bool		operator >= (const T rhs) const { return _value >= rhs; }
		GND constexpr bool		operator <= (const T rhs) const { return _value <= rhs; }

		GND friend bool			operator == (T lhs, Bytes<T> rhs) { return lhs == rhs._value; }
		GND friend bool			operator != (T lhs, Bytes<T> rhs) { return lhs != rhs._value; }
		GND friend bool			operator >  (T lhs, Bytes<T> rhs) { return lhs > rhs._value; }
		GND friend bool			operator <  (T lhs, Bytes<T> rhs) { return lhs < rhs._value; }
		GND friend bool			operator >= (T lhs, Bytes<T> rhs) { return lhs >= rhs._value; }
		GND friend bool			operator <= (T lhs, Bytes<T> rhs) { return lhs <= rhs._value; }

	};

	using BytesU = Bytes<uint64_t>;

	template <typename T>
	inline static constexpr BytesU	SizeOf = BytesU::SizeOf<T>();

	template <typename T>
	inline static constexpr BytesU	AlignOf = BytesU::AlignOf<T>();


	GND constexpr BytesU  operator "" _b(unsigned long long value) { return BytesU(CheckCast<BytesU::Value_t>(value)); }
	GND constexpr BytesU  operator "" _Kb(unsigned long long value) { return BytesU::FromKb(CheckCast<BytesU::Value_t>(value)); }
	GND constexpr BytesU  operator "" _Mb(unsigned long long value) { return BytesU::FromMb(CheckCast<BytesU::Value_t>(value)); }
	GND constexpr BytesU  operator "" _Gb(unsigned long long value) { return BytesU::FromGb(CheckCast<BytesU::Value_t>(value)); }

}








