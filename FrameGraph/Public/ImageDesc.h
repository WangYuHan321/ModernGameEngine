#pragma once

#include "./FrameGraph.h"
#include "../STL/Math/Vec.h"

namespace FrameGraph
{
	//
	//   Image Desc
	//
	struct ImageDesc
	{
		//variables
		EImageDim imageType = EImageDim::Unknown;
		EImage viewType = EImage::Unknown;
		EImageFlags flags = EImageFlags::Unknown;
		uint3 dimension;
		EPixelFormat format = EPixelFormat::Unknown;
		EImageUsage			usage = Default;
		ImageLayer			arrayLayers = 1_layer;
		MipmapLevel			maxLevel = 1_mipmap;
		MultiSamples		samples;				// if > 1 then enabled multisampling
		EQueueUsage			queues = Default;

		bool				isExternal = false;

	};

	//
	// Image View Description
	//

	struct ImageViewDesc
	{
		// variables
		EImage				viewType = Default;
		EPixelFormat		format = Default;
		MipmapLevel			baseLevel;
		uint				levelCount = UMax;
		ImageLayer			baseLayer;
		uint				layerCount = UMax;
		ImageSwizzle		swizzle;
		EImageAspect		aspectMask = EImageAspect::Auto;

		// methods
		ImageViewDesc() {}

		explicit ImageViewDesc(EImage			viewType,
			EPixelFormat	format = Default,
			MipmapLevel		baseLevel = Default,
			uint			levelCount = UMax,
			ImageLayer		baseLayer = Default,
			uint			layerCount = UMax,
			ImageSwizzle	swizzle = Default,
			EImageAspect	aspectMask = EImageAspect::Auto);

		explicit ImageViewDesc(const ImageDesc& desc);

		void Validate(const ImageDesc& desc);

		GND bool operator == (const ImageViewDesc& rhs) const;

		ImageViewDesc& SetType(EImage value) { viewType = value;				return *this; }
		ImageViewDesc& SetFormat(EPixelFormat value) { format = value;				return *this; }
		ImageViewDesc& SetBaseMipmap(uint value) { baseLevel = MipmapLevel{ value };	return *this; }
		ImageViewDesc& SetLevels(uint base, uint count) { baseLevel = MipmapLevel{ base };	levelCount = count;  return *this; }
		ImageViewDesc& SetBaseLayer(uint value) { baseLayer = ImageLayer{ value };	return *this; }
		ImageViewDesc& SetArrayLayers(uint base, uint count) { baseLayer = ImageLayer{ base };		layerCount = count;  return *this; }
		ImageViewDesc& SetSwizzle(ImageSwizzle value) { swizzle = value;				return *this; }
		ImageViewDesc& SetAspect(EImageAspect value) { aspectMask = value;				return *this; }
	};
}