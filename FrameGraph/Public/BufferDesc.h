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
		size_t				offset{ 0 };
		size_t				size{ ~size_t(0) };		// 默认 = “整段剩余 buffer”，与原版 ~0_b 语义一致

		// methods
		BufferViewDesc() {}

		BufferViewDesc(EPixelFormat	format,
			size_t			offset,
			size_t			size) :
			format{ format }, offset{ offset }, size{ size } {
		}

		// 与原版 FrameGraph 一致：offset 夹到 [0, size-1]，size 夹到剩余可用字节
		void Validate(const BufferDesc& desc)
		{
			offset = offset < (desc.size - 1) ? offset : (desc.size - 1);
			size = size < (desc.size - offset) ? size : (desc.size - offset);
		}

		GND bool operator == (const BufferViewDesc& rhs) const
		{
			return	(format == rhs.format)	&
					(offset == rhs.offset)	&
					(size == rhs.size);
		}
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