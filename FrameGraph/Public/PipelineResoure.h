#pragma once

#include "FrameGraph.h"
#include "../STL/Math/Bytes.h"
#include "../STL/Containers/FixedArray.h"
#include "../STL/Containers/FixedMap.h"
#include "../STL/Containers/Ptr.h"

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
				Local::RawSamplerID samplerId;
				ImageViewDesc imageDesc;
				bool hasDesc;
			};

			BindingIndex index;
			EResourceState state;
			EImageSampler samplerType;
			const uint16_t elementCapacity;
			uint16_t elementCount;
			Element elements[1];
		};

		struct Sampler
		{
			static constexpr EDescriptorType typeId = EDescriptorType::Sampler;
			struct Element {
				Local::RawSamplerID samplerId;
			};

			BindingIndex index;
			const uint16_t elementCapacity;
			uint16_t elementCount;
			Element elements[1];
		};

		using CachedID = Atomic<Local::RawPipelineResourcesID::Value_t>;
		using DeallocatorFn_t = void (*) (void*, void*, BytesU);

		struct Uniform
		{
			Local::UniformID id;
			EDescriptorType resType = {};
			uint16_t offset = 0;

			GND bool  operator == (const Local::UniformID& rhs) const { return id == rhs; }
			GND bool  operator <  (const Local::UniformID& rhs) const { return id < rhs; }
		};

		struct DynamicData
		{
			Local::RawDescriptorSetLayoutID layoutId;
			uint						uniformCount = 0;
			uint						uniformsOffset = 0;
			uint						dynamicOffsetsCount = 0;
			uint						dynamicOffsetsOffset = 0;
			BytesU						memSize;

			DynamicData() {}

			GND Uniform* Uniforms() { return Cast<Uniform>(this + BytesU{ uniformsOffset }); }
			GND Uniform	const* Uniforms()			const { return Cast<Uniform>(this + BytesU{ uniformsOffset }); }
			GND uint* DynamicOffsets() { return Cast<uint>(this + BytesU{ dynamicOffsetsOffset }); }

			GND HashVal			CalcHash() const;
			GND bool			operator == (const DynamicData&) const;

			template <typename T, typename Fn>	static void  _ForEachUniform(T&&, Fn&&);
			template <typename Fn>				void ForEachUniform(Fn&& fn) { _ForEachUniform(*this, fn); }
			template <typename Fn>				void ForEachUniform(Fn&& fn) const { _ForEachUniform(*this, fn); }
		};

		struct DynamicDataDeleter
		{
			constexpr DynamicDataDeleter() {}

			DynamicDataDeleter(const DynamicDataDeleter&) {}

			void operator () (DynamicData*) const;
		};

		using DynamicDataPtr = std::unique_ptr< DynamicData, DynamicDataDeleter >;


	private:
		DynamicDataPtr			_dataPtr;
		bool					_allowEmptyResources = false;
		mutable CachedID		_cachedId;



		using PipelineResourceSet = FixedMap<Local::DescriptorSetID, Ptr<const PipelineResources>, GFG_MaxDescriptorSets>;
	};


}