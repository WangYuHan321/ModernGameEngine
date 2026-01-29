#pragma once

#include "FrameGraph.h"
#include "../STL/Math/Bytes.h"
#include "../STL/Containers/FixedArray.h"
#include "../STL/Containers/FixedMap.h"


namespace FrameGraph
{
	//
	//   Pipeline Resources
	//

	struct PipelineResources final
	{
		friend class PipelineResourceHelper;

		//types
	public:
		using Self = PipelineResources;

		enum class EDescriptorType : uint16_t
		{
			Unknown = 0,
			Buffer,
			TexelBuffer,
			Image,
			Texture,
			SubpassInput,
			Sampler,
			RayTracingScene
		};

		struct Buffer
		{
			static constexpr EDescriptorType TypeId = EDescriptorType::Buffer;
		
			struct Element
			{
				Local::RawBufferID bufferID;
				BytesU             offset;
				BytesU             size;
			};

			BindingIndex index;
			EResourceState state;
			uint  dynamicOffsetIndex;
			BytesU staticSize;
			BytesU arrayStride;
			const uint16_t elementCapacity;
			uint16_t elementCount;
			Element elements[1];
		};

		struct TexelBuffer
		{
			static constexpr EDescriptorType TypeId = EDescriptorType::TexelBuffer;

			struct Element
			{
				Local::RawBufferID bufferID;
				BufferViewDesc desc;
			};

			BindingIndex index;
			EResourceState state;
			const uint16_t elementCapacity;
			uint16_t elementCount;
			Element elements[1];
		};

		struct Image
		{
			static constexpr EDescriptorType TypeId = EDescriptorType::Image;

			struct Element
			{
				Local::RawImageID imageId;
				ImageViewDesc desc;
				bool hasDesc;
			};

			BindingIndex index;
			EResourceState state;
			EImageSampler imageType;
			const uint16_t elementCapacity;
			uint16_t elementCount;
			Element elements[1];
		};

		struct Texture
		{
			static constexpr EDescriptorType TypeId = EDescriptorType::Texture;

			struct Element
			{
				Local::RawImageID imageId;
			};

		};


		using PipelineResourceSet = FixedMap<DescriptorSetID, Ptr<const PipelineResources>, GFG_MaxDescriptorSets>;
	};


}