#pragma once
#include "../STL/Algorithms/Hash.h"
#include "../STL/CompileTime/Hash.h"

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
		//Resource ID
		//
		template<uint UID>
		struct ResourceID
		{
			//types
		public:
			using Self = ResourceID<UID>;
			using Index_t = uint16_t;
			using InstanceID_t = uint16_t;
			using Value_t = uint32_t;

			//variables
		private:
			Value_t _value = {};

			static_assert(sizeof(_value) == (sizeof(Index_t) + sizeof(InstanceID_t)));

			static constexpr Index_t _IndexMask = (1 << sizeof(Index_t) * 8) - 1;//0xFFFFFFFFFF
			static constexpr Value_t _InstOffset = sizeof(Index_t) * 8;

			//method

			constexpr ResourceID() {}
			constexpr ResourceID(const ResourceID& other) : _value{ other._value } {}
			explicit constexpr ResourceID(Value_t data) : _value{ data } {}
			explicit constexpr ResourceID(Index_t val, InstanceID_t inst) : _value{ Value_t(val) | (Value_t(inst) << _InstOffset) } {}

			GND constexpr bool			IsValid()						const { return _value != {}; }
			GND constexpr Index_t		Index()						const { return _value & _IndexMask; }
			GND constexpr InstanceID_t	InstanceID()					const { return _value >> _InstOffset; }
			GND constexpr HashVal		GetHash()						const { return HashOf(_value) + HashVal{ UID }; }
			GND constexpr Value_t		Data()							const { return _value; }

			GND constexpr bool			operator == (const Self& rhs)	const { return _value == rhs._value; }
			GND constexpr bool			operator != (const Self& rhs)	const { return not (*this == rhs); }

			GND explicit constexpr		operator bool()				const { return IsValid(); }

			GND static constexpr uint	GetUID() { return UID; }


		};

		enum class RenderTargetID : uint
		{
			Color_0 = 0,
			Color_1 = 1,
			Color_2 = 2,
			Color_3 = 3,
			_LastColor = FG_MaxColorBuffer - 1,
			DepthStencil = FG_MaxColorBuffer,
			Depth = DepthStencil,
			Unknown = ~0u
		};

		static_assert(uint(RenderTargetID::_LastColor) <= FG_MaxColorBuffers);

		using UniformID = FrameGraph::Local::IDWithString<32, 1, false>;
		using PushConstantID = FrameGraph::Local::IDWithString<32, 1, false>;
		using RawBufferID = FrameGraph::Local::ResourceID< 1 >;

	}

}