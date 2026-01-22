#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "../STL/Common.h"

namespace FrameGraph
{
	//
	//   FrameGraph 接口
	//

	enum class EQueueType : uint
	{
		Graphics,           // 支持compute 和 transfer 指令
		AsyncCompute,       // 分离compute queue
		AsyncTransfer,      // 分离transfer queue
		_Count,
		Unknow = ~0u,
	};

	enum class EQueueUsage : uint
	{
		Unknown = 0,
		Graphics = 1 << uint(EQueueType::Graphics),
		AsyncCompute = 1 << uint(EQueueType::AsyncCompute),
		AsyncTransfer = 1 << uint(EQueueType::AsyncTransfer),
		_Last,
		All = ((_Last - 1) << 1) - 1,
	};

	enum class EBufferUsage : uint
	{
		TransferSrc           = 1 << 0,
		TransferDst			  = 1 << 1,
		UniformTexel          = 1 << 2,
		StorageTexel          = 1 << 3,
		Uniform				  = 1 << 4,
		Storage				  = 1 << 5,
		Index				  = 1 << 6,
		Vertex                = 1 << 7,
		Indirect              = 1 << 8,
		RayTracing			  = 1 << 9,

		VertexPplnStore       = 1<< 11,
		FragmentPplnStore     = 1 << 12,
		StorageTexelAtomic    = 1 << 13,
		_Last,
		
		All = ((_Last - 1) << 1) - 1,
		Transfer = TransferDst | TransferSrc,
		Unknown = 0,
	};

	enum class EImageDim : uint8_t
	{
		_1D,
		_2D,
		_3D,
		OneDim = _1D,
		TwoDim = _2D,
		ThreeDim = _3D,
		Unknown = uint8_t(~0u),
	};

	static constexpr auto	EImageDim_1D = EImageDim::_1D;
	static constexpr auto	EImageDim_2D = EImageDim::_2D;
	static constexpr auto	EImageDim_3D = EImageDim::_3D;

	enum class EImage : uint8_t
	{
		_1D,
		_2D,
		_3D,
		_1DArray,
		_2DArray,
		Cube,
		CubeArray,
		OneDim = _1D,
		TwoDim = _2D,
		ThreeDim = _3D,
		OneDimArray = _1DArray,
		TwoDimArray = _2DArray,
		Unknown = uint8_t(~0u),
	};

	static constexpr auto	EImage_1D = EImage::_1D;
	static constexpr auto	EImage_2D = EImage::_2D;
	static constexpr auto	EImage_3D = EImage::_3D;
	static constexpr auto	EImage_1DArray = EImage::_1DArray;
	static constexpr auto	EImage_2DArray = EImage::_2DArray;
	static constexpr auto	EImage_Cube = EImage::Cube;
	static constexpr auto	EImage_CubeArray = EImage::CubeArray;

	enum class EImageFlags : uint8_t
	{
		CubeCompatible = 1 << 0,	// allows to create CubeMap and CubeMapArray from 2D Array
		MutableFormat = 1 << 1,	// allows to change image format
		Array2DCompatible = 1 << 2,	// allows to create 2D Array view from 3D image
		BlockTexelViewCompatible = 1 << 3,	// allows to create view with uncompressed format for compressed image
		_Last,

		Unknown = 0,
	};

	enum class EImageUsage : uint
	{
		TransferSrc = 1 << 0,		// for all copy operations
		TransferDst = 1 << 1,		// for all copy operations
		Sampled = 1 << 2,		// access in shader as texture
		Storage = 1 << 3,		// access in shader as image
		ColorAttachment = 1 << 4,		// color or resolve attachment
		DepthStencilAttachment = 1 << 5,		// depth/stencil attachment
		TransientAttachment = 1 << 6,		// color, resolve, depth/stencil, input attachment
		InputAttachment = 1 << 7,		// input attachment in shader
		ShadingRate = 1 << 8,
		//FragmentDensityMap	= 1 << 9,		// not supported yet

		// special flags for IsSupported() method
		StorageAtomic = 1 << 10,		// same as 'Storage', atomic operations on image
		ColorAttachmentBlend = 1 << 11,		// same as 'ColorAttachment', blend operations on render target
		SampledMinMax = 1 << 13,		// same as 'Sampled'
		_Last,

		All = ((_Last - 1) << 1) - 1,
		Transfer = TransferSrc | TransferDst,
		Unknown = 0,
	};

	enum class EImageAspect : unsigned int
	{
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2,
		Metadata = 1 << 3,
		_Last,

		DepthStencil = Depth | Stencil,
		Auto = ~0u,
		Unknown = 0,
	};

	enum class EPixelFormat : uint
	{
		// signed normalized
		RGBA16_SNorm,
		RGBA8_SNorm,
		RGB16_SNorm,
		RGB8_SNorm,
		RG16_SNorm,
		RG8_SNorm,
		R16_SNorm,
		R8_SNorm,

		// unsigned normalized
		RGBA16_UNorm,
		RGBA8_UNorm,
		RGB16_UNorm,
		RGB8_UNorm,
		RG16_UNorm,
		RG8_UNorm,
		R16_UNorm,
		R8_UNorm,
		RGB10_A2_UNorm,
		RGBA4_UNorm,
		RGB5_A1_UNorm,
		RGB_5_6_5_UNorm,

		// BGRA
		BGR8_UNorm,
		BGRA8_UNorm,

		// sRGB
		sRGB8,
		sRGB8_A8,
		sBGR8,
		sBGR8_A8,

		// signed integer
		R8I,
		RG8I,
		RGB8I,
		RGBA8I,
		R16I,
		RG16I,
		RGB16I,
		RGBA16I,
		R32I,
		RG32I,
		RGB32I,
		RGBA32I,

		// unsigned integer
		R8U,
		RG8U,
		RGB8U,
		RGBA8U,
		R16U,
		RG16U,
		RGB16U,
		RGBA16U,
		R32U,
		RG32U,
		RGB32U,
		RGBA32U,
		RGB10_A2U,

		// float
		R16F,
		RG16F,
		RGB16F,
		RGBA16F,
		R32F,
		RG32F,
		RGB32F,
		RGBA32F,
		RGB_11_11_10F,

		// depth stencil
		Depth16,
		Depth24,
		Depth32F,
		Depth16_Stencil8,
		Depth24_Stencil8,
		Depth32F_Stencil8,

		// compressed
		BC1_RGB8_UNorm,
		BC1_sRGB8,
		BC1_RGB8_A1_UNorm,
		BC1_sRGB8_A1,
		BC2_RGBA8_UNorm,
		BC2_sRGB8_A8,
		BC3_RGBA8_UNorm,
		BC3_sRGB8,
		BC4_R8_SNorm,
		BC4_R8_UNorm,
		BC5_RG8_SNorm,
		BC5_RG8_UNorm,
		BC7_RGBA8_UNorm,
		BC7_sRGB8_A8,
		BC6H_RGB16F,
		BC6H_RGB16UF,
		ETC2_RGB8_UNorm,
		ECT2_sRGB8,
		ETC2_RGB8_A1_UNorm,
		ETC2_sRGB8_A1,
		ETC2_RGBA8_UNorm,
		ETC2_sRGB8_A8,
		EAC_R11_SNorm,
		EAC_R11_UNorm,
		EAC_RG11_SNorm,
		EAC_RG11_UNorm,
		ASTC_RGBA_4x4,
		ASTC_RGBA_5x4,
		ASTC_RGBA_5x5,
		ASTC_RGBA_6x5,
		ASTC_RGBA_6x6,
		ASTC_RGBA_8x5,
		ASTC_RGBA_8x6,
		ASTC_RGBA_8x8,
		ASTC_RGBA_10x5,
		ASTC_RGBA_10x6,
		ASTC_RGBA_10x8,
		ASTC_RGBA_10x10,
		ASTC_RGBA_12x10,
		ASTC_RGBA_12x12,
		ASTC_sRGB8_A8_4x4,
		ASTC_sRGB8_A8_5x4,
		ASTC_sRGB8_A8_5x5,
		ASTC_sRGB8_A8_6x5,
		ASTC_sRGB8_A8_6x6,
		ASTC_sRGB8_A8_8x5,
		ASTC_sRGB8_A8_8x6,
		ASTC_sRGB8_A8_8x8,
		ASTC_sRGB8_A8_10x5,
		ASTC_sRGB8_A8_10x6,
		ASTC_sRGB8_A8_10x8,
		ASTC_sRGB8_A8_10x10,
		ASTC_sRGB8_A8_12x10,
		ASTC_sRGB8_A8_12x12,

		_Count,
		Unknown = ~0u,
	};

}