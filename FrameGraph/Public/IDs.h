#pragma once

#include "../STL/Common.h"

namespace FrameGraph
{

	namespace Local
	{
		//
		//字符串转换ID
		//
		template <size_t Size, uint UID, bool Optimize, uint Seed >
		struct IDWithString
		{
			//types
			using Self = IDWithString<Size, UID, Optimize, Seed>;

		private:

			HashVal _hash;


		public:
			constexpr IDWithString() : _hash{  } {}
			explicit constexpr IDWithString(HashVal hash) : _hash{ hash } {}
			explicit constexpr IDWithString(StringView name) : _hash{ CT_Hash(name.data(), name.length(), Seed) } {}
			explicit constexpr IDWithString(const char* name) : _hash{ CT_Hash(name, UMax, Seed) } {}

			GND constexpr bool operator == (const Self& rhs) const { return _hash == rhs._hash; }
			GND constexpr bool operator != (const Self& rhs) const { return not (*this == rhs); }
			GND constexpr bool operator >  (const Self& rhs) const { return _hash > rhs._hash; }
			GND constexpr bool operator <  (const Self& rhs) const { return rhs > *this; }
			GND constexpr bool operator >= (const Self& rhs) const { return not (*this < rhs); }
			GND constexpr bool operator <= (const Self& rhs) const { return not (*this > rhs); }

			GND constexpr HashVal		GetHash()			const { return _hash; }
			GND constexpr bool			IsDefined()		const { return _hash != _emptyHash; }
			GND constexpr static bool	IsOptimized() { return true; }
			GND constexpr static uint	GetSeed() { return Seed; }
		};

		//
		//ID with string
		//




	}

}