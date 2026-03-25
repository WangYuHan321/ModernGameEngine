#include "VertexInputState.h"

bool FrameGraph::VertexInputState::VertexAttrib::operator==(const VertexAttrib& rhs) const
{
	return true;
}

FrameGraph::VertexInputState::VertexInput::VertexInput()
{
}

FrameGraph::VertexInputState::VertexInput::VertexInput(EVertexType type, Bytes<uint> offset, uint bufferBinding)
{
}

FrameGraph::EVertexType FrameGraph::VertexInputState::VertexInput::ToDstType() const
{
	return FrameGraph::EVertexType();
}

bool FrameGraph::VertexInputState::VertexInput::operator==(const VertexInput& rhs) const
{
	return false;
}

FrameGraph::VertexInputState::BufferBinding::BufferBinding()
{
}

FrameGraph::VertexInputState::BufferBinding::BufferBinding(uint index, Bytes<uint> stride, EVertexInputRate rate)
{
}

bool FrameGraph::VertexInputState::BufferBinding::operator==(const BufferBinding& rhs) const
{
	return false;
}

FrameGraph::VertexInputState::Self& FrameGraph::VertexInputState::Add(const VertexID& id, EVertexType type, BytesU offset, const VertexBufferID& bufferId)
{
	return *this;
	// TODO: insert return statement here
}

FrameGraph::VertexInputState::Self& FrameGraph::VertexInputState::Bind(const VertexBufferID& bufferId, Bytes<uint> stride, uint index, EVertexInputRate rate)
{
	// TODO: insert return statement here
	return *this;
}

FrameGraph::VertexInputState::Self& FrameGraph::VertexInputState::Bind(const VertexBufferID& bufferId, BytesU stride, uint index, EVertexInputRate rate)
{
	// TODO: insert return statement here
	return *this;
}

bool FrameGraph::VertexInputState::ApplyAttribs(ArrayView<VertexAttrib> attribs)
{
	return false;
}

bool FrameGraph::VertexInputState::operator==(const VertexInputState& rhs) const
{
	return false;
}
