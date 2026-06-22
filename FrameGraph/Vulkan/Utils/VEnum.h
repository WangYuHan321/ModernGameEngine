#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	enum class EQueueFamily : uint
	{
		_Count = 31,

		External = VK_QUEUE_FAMILY_EXTERNAL,
		Foreign = VK_QUEUE_FAMILY_FOREIGN_EXT,
		Ignored = VK_QUEUE_FAMILY_IGNORED,
		Unknown = Ignored,
	};

	enum class ExeOrderIndex : uint
	{
		Initial = 0,
		First = 1,
		Final = 0x80000000,
		Unknown = ~0u,
	};

	enum class EQueueFamilyMask : uint
	{
		All = ~0u,
		Unknown = 0,
	};
	
	FG_BIT_OPERATORS(EQueueFamilyMask);


	//
	// Enum Cast (FrameGraph -> Vulkan)
	//

	GND inline VkBufferUsageFlags  VEnumCast(EBufferUsage usage)
	{
		VkBufferUsageFlags result = 0;

		if (uint(usage & EBufferUsage::TransferSrc))		result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (uint(usage & EBufferUsage::TransferDst))		result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (uint(usage & EBufferUsage::UniformTexel))		result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		if (uint(usage & (EBufferUsage::StorageTexel | EBufferUsage::StorageTexelAtomic)))
															result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		if (uint(usage & EBufferUsage::Uniform))			result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (uint(usage & (EBufferUsage::Storage | EBufferUsage::VertexPplnStore | EBufferUsage::FragmentPplnStore)))
															result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (uint(usage & EBufferUsage::Index))				result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (uint(usage & EBufferUsage::Vertex))				result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (uint(usage & EBufferUsage::Indirect))			result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
#ifdef VK_NV_ray_tracing
		if (uint(usage & EBufferUsage::RayTracing))			result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
#endif
		return result;
	}

	// EPixelFormat -> VkFormat
	// 这里覆盖全部非压缩 / 深度 / 打包格式（texel buffer 视图只会用到这些）；
	// 压缩格式（BC/ETC/ASTC/EAC）此处返回 VK_FORMAT_UNDEFINED。
	GND inline VkFormat  VEnumCast(EPixelFormat fmt)
	{
		switch (fmt)
		{
		// signed normalized
		case EPixelFormat::RGBA16_SNorm:	return VK_FORMAT_R16G16B16A16_SNORM;
		case EPixelFormat::RGBA8_SNorm:		return VK_FORMAT_R8G8B8A8_SNORM;
		case EPixelFormat::RGB16_SNorm:		return VK_FORMAT_R16G16B16_SNORM;
		case EPixelFormat::RGB8_SNorm:		return VK_FORMAT_R8G8B8_SNORM;
		case EPixelFormat::RG16_SNorm:		return VK_FORMAT_R16G16_SNORM;
		case EPixelFormat::RG8_SNorm:		return VK_FORMAT_R8G8_SNORM;
		case EPixelFormat::R16_SNorm:		return VK_FORMAT_R16_SNORM;
		case EPixelFormat::R8_SNorm:		return VK_FORMAT_R8_SNORM;
		// unsigned normalized
		case EPixelFormat::RGBA16_UNorm:	return VK_FORMAT_R16G16B16A16_UNORM;
		case EPixelFormat::RGBA8_UNorm:		return VK_FORMAT_R8G8B8A8_UNORM;
		case EPixelFormat::RGB16_UNorm:		return VK_FORMAT_R16G16B16_UNORM;
		case EPixelFormat::RGB8_UNorm:		return VK_FORMAT_R8G8B8_UNORM;
		case EPixelFormat::RG16_UNorm:		return VK_FORMAT_R16G16_UNORM;
		case EPixelFormat::RG8_UNorm:		return VK_FORMAT_R8G8_UNORM;
		case EPixelFormat::R16_UNorm:		return VK_FORMAT_R16_UNORM;
		case EPixelFormat::R8_UNorm:		return VK_FORMAT_R8_UNORM;
		case EPixelFormat::RGB10_A2_UNorm:	return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case EPixelFormat::RGBA4_UNorm:		return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		case EPixelFormat::RGB5_A1_UNorm:	return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		case EPixelFormat::RGB_5_6_5_UNorm:	return VK_FORMAT_R5G6B5_UNORM_PACK16;
		// BGRA
		case EPixelFormat::BGR8_UNorm:		return VK_FORMAT_B8G8R8_UNORM;
		case EPixelFormat::BGRA8_UNorm:		return VK_FORMAT_B8G8R8A8_UNORM;
		// sRGB
		case EPixelFormat::sRGB8:			return VK_FORMAT_R8G8B8_SRGB;
		case EPixelFormat::sRGB8_A8:		return VK_FORMAT_R8G8B8A8_SRGB;
		case EPixelFormat::sBGR8:			return VK_FORMAT_B8G8R8_SRGB;
		case EPixelFormat::sBGR8_A8:		return VK_FORMAT_B8G8R8A8_SRGB;
		// signed integer
		case EPixelFormat::R8I:				return VK_FORMAT_R8_SINT;
		case EPixelFormat::RG8I:			return VK_FORMAT_R8G8_SINT;
		case EPixelFormat::RGB8I:			return VK_FORMAT_R8G8B8_SINT;
		case EPixelFormat::RGBA8I:			return VK_FORMAT_R8G8B8A8_SINT;
		case EPixelFormat::R16I:			return VK_FORMAT_R16_SINT;
		case EPixelFormat::RG16I:			return VK_FORMAT_R16G16_SINT;
		case EPixelFormat::RGB16I:			return VK_FORMAT_R16G16B16_SINT;
		case EPixelFormat::RGBA16I:			return VK_FORMAT_R16G16B16A16_SINT;
		case EPixelFormat::R32I:			return VK_FORMAT_R32_SINT;
		case EPixelFormat::RG32I:			return VK_FORMAT_R32G32_SINT;
		case EPixelFormat::RGB32I:			return VK_FORMAT_R32G32B32_SINT;
		case EPixelFormat::RGBA32I:			return VK_FORMAT_R32G32B32A32_SINT;
		// unsigned integer
		case EPixelFormat::R8U:				return VK_FORMAT_R8_UINT;
		case EPixelFormat::RG8U:			return VK_FORMAT_R8G8_UINT;
		case EPixelFormat::RGB8U:			return VK_FORMAT_R8G8B8_UINT;
		case EPixelFormat::RGBA8U:			return VK_FORMAT_R8G8B8A8_UINT;
		case EPixelFormat::R16U:			return VK_FORMAT_R16_UINT;
		case EPixelFormat::RG16U:			return VK_FORMAT_R16G16_UINT;
		case EPixelFormat::RGB16U:			return VK_FORMAT_R16G16B16_UINT;
		case EPixelFormat::RGBA16U:			return VK_FORMAT_R16G16B16A16_UINT;
		case EPixelFormat::R32U:			return VK_FORMAT_R32_UINT;
		case EPixelFormat::RG32U:			return VK_FORMAT_R32G32_UINT;
		case EPixelFormat::RGB32U:			return VK_FORMAT_R32G32B32_UINT;
		case EPixelFormat::RGBA32U:			return VK_FORMAT_R32G32B32A32_UINT;
		case EPixelFormat::RGB10_A2U:		return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		// float
		case EPixelFormat::R16F:			return VK_FORMAT_R16_SFLOAT;
		case EPixelFormat::RG16F:			return VK_FORMAT_R16G16_SFLOAT;
		case EPixelFormat::RGB16F:			return VK_FORMAT_R16G16B16_SFLOAT;
		case EPixelFormat::RGBA16F:			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case EPixelFormat::R32F:			return VK_FORMAT_R32_SFLOAT;
		case EPixelFormat::RG32F:			return VK_FORMAT_R32G32_SFLOAT;
		case EPixelFormat::RGB32F:			return VK_FORMAT_R32G32B32_SFLOAT;
		case EPixelFormat::RGBA32F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case EPixelFormat::RGB_11_11_10F:	return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		// depth / stencil
		case EPixelFormat::Depth16:			return VK_FORMAT_D16_UNORM;
		case EPixelFormat::Depth24:			return VK_FORMAT_X8_D24_UNORM_PACK32;
		case EPixelFormat::Depth32F:		return VK_FORMAT_D32_SFLOAT;
		case EPixelFormat::Depth16_Stencil8:	return VK_FORMAT_D16_UNORM_S8_UINT;
		case EPixelFormat::Depth24_Stencil8:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case EPixelFormat::Depth32F_Stencil8:	return VK_FORMAT_D32_SFLOAT_S8_UINT;
		default:
			ASSERT(false && "unsupported / compressed pixel format");
			return VK_FORMAT_UNDEFINED;
		}
	}

	GND inline VkAccessFlags  EResourceState_ToAccess(EResourceState state)
	{
		const uint access = uint(state) & uint(EResourceState::_AccessMask);
		const bool read   = (uint(state) & uint(EResourceState::_Read)) != 0;
		const bool write  = (uint(state) & uint(EResourceState::_Write)) != 0;

		switch (EResourceState(access))
		{
		case EResourceState::_Access_ShaderStorage:
			return (read ? VK_ACCESS_SHADER_READ_BIT : 0) | (write ? VK_ACCESS_SHADER_WRITE_BIT : 0);
		case EResourceState::_Access_Uniform:
			return VK_ACCESS_UNIFORM_READ_BIT;
		case EResourceState::_Access_ShaderSample:
			return VK_ACCESS_SHADER_READ_BIT;
		case EResourceState::_Access_InputAttachment:
			return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case EResourceState::_Access_Transfer:
			return (read ? VK_ACCESS_TRANSFER_READ_BIT : 0) | (write ? VK_ACCESS_TRANSFER_WRITE_BIT : 0);
		case EResourceState::_Access_ColorAttachment:
			return (read ? VK_ACCESS_COLOR_ATTACHMENT_READ_BIT : 0) | (write ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : 0);
		case EResourceState::_Access_DepthStencilAttachment:
			return (read ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : 0) | (write ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);
		case EResourceState::_Access_Host:
			return (read ? VK_ACCESS_HOST_READ_BIT : 0) | (write ? VK_ACCESS_HOST_WRITE_BIT : 0);
		case EResourceState::_Access_IndirectBuffer:
			return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		case EResourceState::_Access_IndexBuffer:
			return VK_ACCESS_INDEX_READ_BIT;
		case EResourceState::_Access_VertexBuffer:
			return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		case EResourceState::_Access_Present:
		default:
			return 0;
		}
	}

	GND inline VkPipelineStageFlags  _EResourceState_ShaderStages(EResourceState state)
	{
		const uint			 s = uint(state);
		VkPipelineStageFlags result = 0;

		if (s & uint(EResourceState::_VertexShader))			result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if (s & uint(EResourceState::_TessControlShader))		result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
		if (s & uint(EResourceState::_TessEvaluationShader))	result |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
		if (s & uint(EResourceState::_GeometryShader))			result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
		if (s & uint(EResourceState::_FragmentShader))			result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (s & uint(EResourceState::_ComputeShader))			result |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef VK_NV_mesh_shader
		if (s & uint(EResourceState::_MeshTaskShader))			result |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV;
		if (s & uint(EResourceState::_MeshShader))				result |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV;
#endif
#ifdef VK_NV_ray_tracing
		if (s & uint(EResourceState::_RayTracingShader))		result |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
#endif
		return result;
	}

	GND inline VkPipelineStageFlags  EResourceState_ToStage(EResourceState state)
	{
		// 与原版 EResourceState_ToPipelineStages 的映射保持一致
		switch (EResourceState(uint(state) & uint(EResourceState::_AccessMask)))
		{
		case EResourceState::Unknown:						return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		case EResourceState::_Access_InputAttachment:		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		case EResourceState::_Access_Transfer:				return VK_PIPELINE_STAGE_TRANSFER_BIT;
		case EResourceState::_Access_ColorAttachment:		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		case EResourceState::_Access_Present:				return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		case EResourceState::_Access_IndirectBuffer:		return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		case EResourceState::_Access_Host:					return VK_PIPELINE_STAGE_HOST_BIT;
		case EResourceState::_Access_IndexBuffer:
		case EResourceState::_Access_VertexBuffer:			return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

		case EResourceState::_Access_DepthStencilAttachment:
		{
			VkPipelineStageFlags result = 0;
			if (uint(state) & uint(EResourceState::EarlyFragmentTests))	result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			if (uint(state) & uint(EResourceState::LateFragmentTests))	result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			return result != 0 ? result : (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
		}

		case EResourceState::_Access_ShaderStorage:
		case EResourceState::_Access_Uniform:
		case EResourceState::_Access_ShaderSample:
		default:
			break;
		}

		// ShaderStorage / Uniform / ShaderSample：由 shader stage 位推导
		const VkPipelineStageFlags stages = _EResourceState_ShaderStages(state);
		return stages != 0 ? stages : VkPipelineStageFlags(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	}

	// EMemoryType -> VkMemoryPropertyFlags
	GND inline VkMemoryPropertyFlags  VMemoryTypeToFlags(EMemoryType type)
	{
		VkMemoryPropertyFlags flags = 0;

		if (uint(type) == 0)
			flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		if (uint(type & EMemoryType::HostRead))
			flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

		if (uint(type & EMemoryType::HostWrite))
			flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		if (flags == 0)
			flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		return flags;
	}

}

