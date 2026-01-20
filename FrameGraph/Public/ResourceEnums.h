#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

namespace FrameGraph
{
	//
	//   FrameGraph 接口
	//

	enum class EBufferUsage : unsigned int
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

	enum class EImageUsage : unsigned int
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
}