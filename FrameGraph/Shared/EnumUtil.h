#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Math/Bytes.h"
#include "../Public/VertexEnums.h"
#include "../Public/ShaderEnums.h"
#include "../Public/EResourceState.h"

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


	GND inline EResourceState  EResourceState_FromShaderAccess(EShaderAccess access)
	{
		switch (access) {
		case EShaderAccess::ReadOnly:		return EResourceState::ShaderRead;
		case EShaderAccess::WriteOnly:		return EResourceState::ShaderWrite;
		case EShaderAccess::ReadWrite:		return EResourceState::ShaderReadWrite;
		case EShaderAccess::WriteDiscard:	return EResourceState::ShaderWrite | EResourceState::InvalidateBefore;
		}
	}

}