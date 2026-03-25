#pragma once
#include "../STL/CompileTime/Hash.h"
#include "../STL/Algorithms/Hash.h"
#include "../STL/Containers/StringView.h"
#include "../Public/Config.h"

namespace FrameGraph
{

	namespace Local
	{
		//
		//字符串转换ID
		//
		template <size_t Size, uint UID, bool Optimize, uint Seed = 1 >
		struct IDWithString
		{
			//types
			using Self = IDWithString<Size, UID, Optimize, Seed>;

		private:

			HashVal _hash;
			HashVal _emptyHash{};

		public:
			constexpr IDWithString() : _hash{  } {}
			explicit constexpr IDWithString(HashVal hash) : _hash{ hash } {}
			explicit constexpr IDWithString(StringView name) : _hash{ CT_Hash(name.data(), name.length(), Seed) } {}
			explicit constexpr IDWithString(const char* name) : _hash{ CT_Hash(name, {}, Seed) } {}

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

			GND constexpr bool			IsValid()						const { return false; }
			GND constexpr Index_t		Index()						const { return _value & _IndexMask; }
			GND constexpr InstanceID_t	InstanceID()					const { return _value >> _InstOffset; }
			GND constexpr HashVal		GetHash()						const { return HashOf(_value) + HashVal{ UID }; }
			GND constexpr Value_t		Data()							const { return _value; }

			GND constexpr bool			operator == (const Self& rhs)	const { return _value == rhs._value; }
			GND constexpr bool			operator != (const Self& rhs)	const { return not (*this == rhs); }

			GND explicit constexpr		operator bool()				const { return IsValid(); }

			GND static constexpr uint	GetUID() { return UID; }


		};

		template <typename T>
		struct ResourceIDWrap;
		
		template<uint UID>
		struct ResourceIDWrap<ResourceID<UID>>
		{
		public:
			using Self = ResourceIDWrap<ResourceID<UID>>;
			using IDType = ResourceID<UID>;
			
		private:
			IDType _id;
		
		public:
			constexpr ResourceIDWrap() {}
			constexpr ResourceIDWrap(const IDType& id) : _id{ id } {}
			constexpr ResourceIDWrap(const Self& other) : _id{ other._id } {}
			GND constexpr bool operator == (const Self& rhs) const { return _id == rhs._id; }
			GND constexpr bool operator != (const Self& rhs) const { return not (*this == rhs); }
			GND explicit constexpr operator bool() const { return _id.IsValid(); }
		};

		enum class RenderTargetID : uint
		{
			Color_0 = 0,
			Color_1 = 1,
			Color_2 = 2,
			Color_3 = 3,
			_LastColor = GFG_MaxColorBuffers - 1,
			DepthStencil = GFG_MaxColorBuffers,
			Depth = DepthStencil,
			Unknown = ~0u
		};

		static_assert(uint(RenderTargetID::_LastColor) <= GFG_MaxColorBuffers);

	};


	using UniformID = Local::IDWithString<32, 1, false>;
	using PushConstantID = Local::IDWithString<32, 2, false>;
	using DescriptorSetID = Local::IDWithString<32, 4, false>;
	using SpecializationID = Local::IDWithString<32, 5, false>;
	using VertexID = Local::IDWithString<32, 6, false>;
	using VertexBufferID = Local::IDWithString< 32, 7, false >;

	using RawBufferID = Local::ResourceID< 1 >;
	using RawImageID = Local::ResourceID< 2 >;
	using RawGPipelineID = Local::ResourceID< 3 >;
	using RawMPipelineID = Local::ResourceID< 4 >;
	using RawCPipelineID = Local::ResourceID< 5 >;
	using RawRTPipelineID = Local::ResourceID< 6 >;
	using RawSamplerID = Local::ResourceID< 7 >;
	using RawDescriptorSetLayoutID = Local::ResourceID< 8 >;
	using RawPipelineResourcesID = Local::ResourceID< 9 >;
	using LogicalPassID = Local::ResourceID< 10 >;
	using RawRTSceneID = Local::ResourceID< 11 >;
	using RawRTGeometryID = Local::ResourceID< 12 >;
	using RawRTShaderTableID = Local::ResourceID< 13 >;
	using RawSwapchainID = Local::ResourceID< 14 >;

	//strong reference
	using BufferID = Local::ResourceIDWrap < RawBufferID >;
	using ImageID = Local::ResourceIDWrap < RawImageID >;
	using GPipelineID = Local::ResourceIDWrap < RawGPipelineID >;
	using MPipelineID = Local::ResourceIDWrap < RawMPipelineID >;
	using CPipelineID = Local::ResourceIDWrap < RawCPipelineID >;
	using RTPipelineID = Local::ResourceIDWrap< RawRTPipelineID >;
	using SamplerID = Local::ResourceIDWrap< RawSamplerID >;

}