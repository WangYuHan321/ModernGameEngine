#pragma once

#include "FrameGraph.h"
#include "../STL/Math/Bytes.h"

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
			};

			BindingIndex index;
			EResourceState state;
			uint  dynamicOffsetIndex;
			BytesU staticSize;
			BytesU arrayStride;
			const uint16_t elementCapacity;
			uint16_t elementCount;
		};
	};


}