#pragma once

#include "./VertexDesc.h"
#include "./IDs.h"
#include "../STL/Math/Bytes.h"
#include "../STL/Containers/FixedMap.h"
#include "../STL/Containers/ArrayView.h"
namespace FrameGraph
{
	//
	//Vertex Input State
	//
	class VertexInputState
	{
		//types
	public:
		struct VertexAttrib
		{
			Local::VertexID id;
			uint index;
			EVertexType type;

			GND bool operator == (const VertexAttrib& rhs) const;
		};

		using Self = VertexInputState;

		struct VertexInput
		{
			//variables
			EVertexType type;
			uint index;
			Bytes<uint> offset;
			uint bufferBinding;

			// methods
			VertexInput();
			VertexInput(EVertexType type, Bytes<uint> offset, uint bufferBinding);

			GND EVertexType ToDstType() const;

			GND bool  operator == (const VertexInput& rhs) const;
		};

		struct BufferBinding
		{
			// variables
			uint				index;
			Bytes<uint>			stride;
			EVertexInputRate	rate;

			// methods
			BufferBinding();
			BufferBinding(uint index, Bytes<uint> stride, EVertexInputRate rate);

			GND bool  operator == (const BufferBinding& rhs) const;
		};


		using Vertices_t = FixedMap< Local::VertexID, VertexInput, GFG_MaxVertexAttribs >;
		using Bindings_t = FixedMap< Local::VertexBufferID, BufferBinding, GFG_MaxVertexBuffers >;

		friend struct std::hash < VertexInputState::VertexInput >;
		friend struct std::hash < VertexInputState::BufferBinding >;
		friend struct std::hash < VertexInputState >;


		//variables
	private:
		Vertices_t _vertices;
		Bindings_t _bindings;

		// methods
	public:
		VertexInputState() {}

		template <typename ClassType, typename ValueType>
		Self& Add(const Local::VertexID & id, ValueType ClassType::* vertex, const Local::VertexBufferID& bufferId = Default);

		template <typename ClassType, typename ValueType>
		Self& Add(const Local::VertexID& id, ValueType ClassType::* vertex, bool norm, const Local::VertexBufferID& bufferId = Default);

		Self& Add(const Local::VertexID& id, EVertexType type, BytesU offset, const Local::VertexBufferID& bufferId = Default);

		Self& Bind(const Local::VertexBufferID& bufferId, Bytes<uint> stride, uint index = BindingIndex_Auto, EVertexInputRate rate = EVertexInputRate::Vertex);
		Self& Bind(const Local::VertexBufferID& bufferId, BytesU stride, uint index = BindingIndex_Auto, EVertexInputRate rate = EVertexInputRate::Vertex);

		void  Clear();

		bool  ApplyAttribs(ArrayView<VertexAttrib> attribs);

		GND bool	operator == (const VertexInputState& rhs) const;

		GND Vertices_t const& Vertices()			const { return _vertices; }
		GND Bindings_t const& BufferBindings()	const { return _bindings; }

	};


}