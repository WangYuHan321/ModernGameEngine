#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "ResourceEnums.h"

namespace FrameGraph
{
	//
	//   Buffer Desc
	//

	struct BufferDesc
	{
		size_t size;
		EBufferUsage usage = EBufferUsage::Unknown;
		EQueueUsage  queues = EQueueUsage::Unknown;


		BufferDesc() {}
		BufferDesc(size_t size, EBufferUsage usage, EQueueUsage queue) : size(size), usage(usage), queues(queue) {}

	};

	struct BufferViewDesc
	{
		// variables
		EPixelFormat		format = EPixelFormat::Unknown;
		size_t				offset;
		size_t				size{ 0 };

		// methods
		BufferViewDesc() {}

		BufferViewDesc(EPixelFormat	format,
			size_t			offset,
			size_t			size) :
			format{ format }, offset{ offset }, size{ size } {
		}

		void Validate(const BufferDesc& desc) {};

		GND bool operator == (const BufferViewDesc& rhs) const {};
	};

}

namespace std
{
	template <>
	struct hash< FrameGraph::BufferViewDesc >
	{
		GND size_t  operator () (const FrameGraph::BufferViewDesc& value) const
		{
#if FG_FAST_HASH
			return size_t(FGC::HashOf(AddressOf(value), sizeof(value)));
#else
			FrameGraph::HashVal	result;
			result << FrameGraph::HashOf(value.format);
			result << FrameGraph::HashOf(value.offset);
			result << FrameGraph::HashOf(value.size);
			return size_t(result);
#endif
		}
	};
}