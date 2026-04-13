#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Math/Bytes.h"
#include "../Public/VertexEnums.h"
#include "../Public/ShaderEnums.h"

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


	GND inline EShaderDebugMode EShaderDebugMode_FromFlags(EShaderLangFormat value)
	{
		switch (value & EShaderLangFormat::_DebugModeMask)
		{
		//case EShaderLangFormat::Unknown:				return EShaderDebugMode::None;
		case EShaderLangFormat::EnableDebugTrace: return EShaderDebugMode::Trace;
		case EShaderLangFormat::EnableProfiling: return EShaderDebugMode::Profiling;
		case EShaderLangFormat::EnableTimeMap: return EShaderDebugMode::Timemap;
		default:
			return EShaderDebugMode::Unknown;
		}
	}







	
}