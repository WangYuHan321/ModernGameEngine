#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Math/Bytes.h"
#include"../STL/CompileTime/Constants.h"
#include "../Public/Config.h"
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

	GND inline EResourceState  EResourceState_FromShaders(EShaderStage values)
	{
		EResourceState	result = Zero;

		for (EShaderStage t = EShaderStage(1 << 0); t < EShaderStage::_Last; t = EShaderStage(uint(t) << 1))
		{
			switch (t)
			{
			case EShaderStage::Vertex:			result |= EResourceState::_VertexShader;			break;
			case EShaderStage::TessControl:		result |= EResourceState::_TessControlShader;		break;
			case EShaderStage::TessEvaluation:	result |= EResourceState::_TessEvaluationShader;	break;
			case EShaderStage::Geometry:			result |= EResourceState::_GeometryShader;			break;
			case EShaderStage::Fragment:			result |= EResourceState::_FragmentShader;			break;
			case EShaderStage::Compute:			result |= EResourceState::_ComputeShader;			break;
			case EShaderStage::MeshTask:			result |= EResourceState::_MeshTaskShader;			break;
			case EShaderStage::Mesh:				result |= EResourceState::_MeshShader;				break;
			case EShaderStage::RayGen:
			case EShaderStage::RayAnyHit:
			case EShaderStage::RayClosestHit:
			case EShaderStage::RayMiss:
			case EShaderStage::RayIntersection:
			case EShaderStage::RayCallable:		result |= EResourceState::_RayTracingShader;		break;
			case EShaderStage::_Last:
			case EShaderStage::All:
			case EShaderStage::AllGraphics:
			case EShaderStage::AllRayTracing:
			case EShaderStage::Unknown:
			}
		}
		return result;
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