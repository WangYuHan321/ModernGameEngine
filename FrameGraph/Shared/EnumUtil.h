#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Math/Bytes.h"
#include "../Public/VertexEnums.h"

namespace FrameGraph
{
	GND inline BytesU EIndex_SizeOf(EIndex valude)
	{
		switch (valude)
		{
		case EIndex::UShort:	return SizeOf<uint16_t>;
		case EIndex::UInt:		return SizeOf<uint32_t>;
		case EIndex::Unknown:	break;
		}
	}










	
}