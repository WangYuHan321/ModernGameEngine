#pragma once

#include "FrameGraph.h"
#include"../STL/Math/Bytes.h"
#include "../STL/Containers/Union.h"
#include "../STL/Containers/ArrayView.h"
#include "../STL/Containers/StringView.h"
#include "../STL/Containers/FixedMap.h"
#include "../STL/Containers/FixedArray.h"

namespace FrameGraph
{
	//
	//   Pipeline Description
	//

	struct PipelineDescription
	{
		//types
	public:
		static constexpr uint STATIC_OFFSET = {};

		struct Texture
		{
			EResourceState state = {};
			EImageSampler textureType = {};

			GND bool  operator == (const Texture& rhs) const;
		};

		struct Sampler
		{
			GND bool operator ==(const Sampler& rhs) const;
		};

		struct SubpassInput
		{
			EResourceState state = {};
			uint attachmentIndex = {};
			bool isMultisample = false;

			GND bool operator == (const SubpassInput& rhs) const;
		};

		struct Image
		{
			EResourceState state = {};
			EImageSampler textureType = {};

			GND bool  operator == (const Texture& rhs) const;
		};

		struct UniformBuffer
		{
			EResourceState state = {};
			uint dynamicOffsetIndex = STATIC_OFFSET;
			BytesU size;

			GND bool  operator == (const Texture& rhs) const;
		};

		struct StorageBuffer
		{
			/*
			struct Particle {
				glm::vec3 position;    // 12 字节
				glm::vec3 velocity;    // 12 字节
				glm::vec4 color;       // 16 字节
				float size;            // 4 字节
				float lifetime;        // 4 字节
				// 总计: 48 字节
			};

			// 对应的 StorageBuffer 配置
			StorageBuffer particleBuffer;
			particleBuffer.state = EResourceState::_Access_ShaderStorage;
			particleBuffer.staticSize = MAX_PARTICLES * sizeof(Particle);  // 例如：10000 * 48 = 480KB
			particleBuffer.arrayStride = sizeof(Particle);  // 48 字节

			*/

			EResourceState		state = Default;
			uint				dynamicOffsetIndex = STATIC_OFFSET;
			BytesU				staticSize;
			BytesU				arrayStride;

			GND bool  operator == (const StorageBuffer& rhs) const;
		};

		struct RayTracingScene
		{
			EResourceState state = {};

			GND bool  operator == (const RayTracingScene& rhs) const;
		};

		//uniform 
		struct _TextureUniform
		{
			const Local::UniformID id;
			const Texture data;
			const BindingIndex index;
			const uint arraySize;
			const EShaderStage stageFlags;

			_TextureUniform(const Local::UniformID& id, EImageSampler textureType, const BindingIndex& index, uint arraySize, EShaderStage stageFlags);
		};

		struct _SamplerUniform
		{
			const Local::UniformID id;
			const Sampler data;
			const BindingIndex index;
			const uint arraySize;
			const EShaderStage stageFlags;

			_SamplerUniform(const Local::UniformID& id, const BindingIndex& index, uint arraySize, EShaderStages stageFlags);
		};

		struct _SubpassInputUniform
		{
			const Local::UniformID			id;
			const SubpassInput		data;
			const BindingIndex		index;
			const uint				arraySize;
			const EShaderStage		stageFlags;

			_SubpassInputUniform(const Local::UniformID& id, uint attachmentIndex, bool isMultisample, const BindingIndex& index,
				uint arraySize, EShaderStage stageFlags);
		};

		struct _ImageUniform
		{
			const Local::UniformID			id;
			const Image				data;
			const BindingIndex		index;
			const uint				arraySize;
			const EShaderStage		stageFlags;

			_ImageUniform(const Local::UniformID& id, EImageSampler imageType, EShaderAccess access,
				const BindingIndex& index, uint arraySize, EShaderStage stageFlags);
		};

		struct _UBufferUniform
		{
			const Local::UniformID			id;
			const UniformBuffer		data;
			const BindingIndex		index;
			const uint				arraySize;
			const EShaderStage		stageFlags;

			_UBufferUniform(const Local::UniformID& id, BytesU size, const BindingIndex& index, uint arraySize,
				EShaderStage stageFlags, uint dynamicOffsetIndex = STATIC_OFFSET);
		};

		struct _StorageBufferUniform
		{
			const Local::UniformID			id;
			const StorageBuffer		data;
			const BindingIndex		index;
			const uint				arraySize;
			const EShaderStage		stageFlags;

			_StorageBufferUniform(const Local::UniformID& id, BytesU staticSize, BytesU arrayStride, EShaderAccess access, const BindingIndex& index,
				uint arraySize, EShaderStage stageFlags, uint dynamicOffsetIndex = STATIC_OFFSET);
		};

		struct _RayTracingSceneUniform
		{
			const Local::UniformID			id;
			const RayTracingScene	data;
			const BindingIndex		index;
			const uint				arraySize;
			const EShaderStage		stageFlags;

			_RayTracingSceneUniform(const Local::UniformID& id, const BindingIndex& index, uint arraySize, EShaderStage stageFlags);
		};

		struct PushConstant
		{
			EShaderStage stageFlag;
			Bytes<uint16_t>		offset;
			Bytes<uint16_t>		size;

			PushConstant() {}
			PushConstant(EShaderStage stages, BytesU offset, BytesU size) : stageFlag{ stages }, offset{ offset }, size{ size } {}
		};

		struct _PushConstant
		{
			Local::PushConstantID		id;
			PushConstant		data;

			_PushConstant(const Local::PushConstantID& id, EShaderStage stages, BytesU offset, BytesU size) : id{ id }, data{ stages, offset, size } {}
		};

		struct SpecConstant
		{
			Local::SpecializationID id;
			uint index;
		};

		using UniformData_t = Union< NullUnion, Texture, Sampler, SubpassInput, Image, UniformBuffer, StorageBuffer, RayTracingScene >;

		struct Uniform
		{
			UniformData_t data;
			BindingIndex index;
			uint arraySize = 1;
			EShaderStage stageFlag;

			GND bool operator == (const Uniform& rhs) const;
		};

		using UniformMap_t = HashMap<Local::UniformID, Uniform>;
		using UniformMapPtr = SharedPtr<const UniformMap_t>;

		struct DescriptorSet
		{
			Local::DescriptorSetID		id;
			uint				bindingIndex = UMax;
			UniformMapPtr		uniforms;
		};

		using DescriptorSets_t = FixedArray< DescriptorSet, GFG_MaxDescriptorSets >;
		using PushConstants_t = FixedMap< Local::PushConstantID, PushConstant, GFG_MaxPushConstants >;

		struct PipelineLayout
		{
			DescriptorSets_t descriptorSets;
			PushConstants_t pushConstants;
			PipelineLayout() {}
		};

		template<typename T>
		class IShaderData {
		public:
			GND virtual T const& GetData() const = 0;
			GND virtual StringView	GetEntry() const = 0;
			GND virtual StringView	GetDebugName() const = 0;
			GND virtual size_t		GetHashOfData() const = 0;
			virtual bool		ParseDebugOutput(EShaderDebugMode mode, ArrayView<uint8_t> trace, OUT Array<String>& result) const = 0;
		};

		template <typename T>
		using SharedShaderPtr = SharedPtr< IShaderData<T> >;
		using VkShaderPtr = SharedShaderPtr< ShaderModuleVk_t >;
		using SpirvShaderPtr = SharedShaderPtr< Array<uint> >;
		using ShaderSourcePtr = SharedShaderPtr< String >;

		using ShaderDataUnion_t = Union< NullUnion, ShaderSourcePtr, SpirvShaderPtr, VkShaderPtr >;
		using ShaderDataMap_t = HashMap< EShaderLangFormat, ShaderDataUnion_t >;
		using SpecConstants_t = FixedMap< Local::SpecializationID, uint, GFG_MaxSpecConstants >;	// id, index

		struct Shader
		{
			ShaderDataMap_t		data;
			SpecConstants_t		specConstants;

			Shader() {}
			void AddShaderData(EShaderLangFormat fmt, StringView entry, String&& src, StringView dbgName = Default);
			void AddShaderData(EShaderLangFormat fmt, StringView entry, Array<uint>&& bin, StringView dbgName = Default);
		};

		//variables
		public:
			PipelineLayout _pipelineLayout;

		//method
		protected:
			PipelineDescription() {}

			void _AddDescriptorSet(const Local::DescriptorSetID & id,
				uint									index,
				ArrayView< _TextureUniform >			textures,
				ArrayView< _SamplerUniform >			samplers,
				ArrayView< _SubpassInputUniform >		subpassInputs,
				ArrayView< _ImageUniform >				images,
				ArrayView< _UBufferUniform >			uniformBuffers,
				ArrayView< _StorageBufferUniform >		storageBuffers,
				ArrayView< _RayTracingSceneUniform >	rtScenes);

			void _SetPushConstants(ArrayView<_PushConstant> values);
	};

	///
	/// Graphics Pipelie Description
	/// 
	class GraphicsPipelineDesc final : PipelineDescription
	{
		//types
		struct FragmentOutput
		{
			//variables
			uint index = {};
			EFragOutput type = {};

			// methods
			FragmentOutput() {}
			FragmentOutput(uint index, EFragOutput type) : index{ index }, type{ type } {}

			GND bool operator == (const FragmentOutput& rhs) const;
		};

		using Self = GraphicsPipelineDesc;
		using TopologyBits_t = BitSet< uint(EPrimitive::_Count) >;
		using Shaders_t = FixedMap< EShader, Shader, 8 >;
		using VertexAttrib = VertexInputState::VertexAttrib;
		using VertexAttribs_t = FixedArray< VertexAttrib, GFG_MaxVertexAttribs >;
		using FragmentOutputs_t = FixedArray< FragmentOutput, GFG_MaxColorBuffers >;

		//variables
		Shaders_t _shaders;
		TopologyBits_t _supportedTopology;
		






	};


}