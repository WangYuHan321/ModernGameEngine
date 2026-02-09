#pragma once

#include "./IDs.h"
#include "./Pipeline.h"
#include "./Types.h"
#include "ImageSwizzle.h"
#include "MultiSamples.h"
#include "./BufferDesc.h"
#include "./ShaderEnums.h"
#include "./ResourceEnums.h"
#include "./RenderState.h"
#include "./RenderStateEnum.h"
#include "./VertexInputState.h"
#include "./MipmapLevel.h"
#include "./BindingIndex.h"
#include "./ImageLayer.h"
#include "./ImageView.h"
#include "./ImageDesc.h"
#include "./VulkanType.h"
#include "./EResourceState.h"

#include<functional>

namespace FrameGraph
{
	//
	//   FrameGraph 接口
	//
	
	class IFrameGraph : public std::enable_shared_from_this<IFrameGraph>
	{
		//类型
	public:
		using DeviceInfo_t = Union<NullUnion, VulkanDeviceInfo>;//他仅支持2个类型 空类型 和 VulkanDeviceInfo
		using SwapchianCreateInfo_t = Union<NullUnion, VulkanSwapchainCreateInfo>;
		using ExternalImageDesc_t = Union<NullUnion, VulkanImageDesc>;
		using ExternalBufferDesc_t = Union<NullUnion, VulkanBufferDesc>;
		using ExternalImage_t = Union<NullUnion, ImageVk_t>;
		using ExternalBuffer_t = Union< NullUnion, BufferVk_t >;

		using OnExternalImageReleased_t = std::function< void(const ExternalImage_t&)>;
		using OnExternalBufferReleased_t = std::function< void(const ExternalBuffer_t&)>;
		using ShaderDebugCallback_t = std::function< void(StringView taskName, StringView shaderName, EShaderStage, ArrayView<String> output)>;

		//统计
		struct RenderingStatistics
		{
			uint descriptorBinds = 0;
			uint pushConstants = 0;
			uint pipelineBarriers = 0;
			uint transferOps = 0;

			uint indexBufferBindings = 0;
			uint vertexBufferBIndings = 0;
			uint drawCalls = 0;
			uint indirectDrawCalls = 0;
			uint64_t vertexCount = 0;
			uint64_t primitiveCount = 0;
			uint graphicsPipelineBindings = 0;
			uint dynamicStateChanges = 0;

			uint dispatchCalls = 0;
			uint computePipelineBinding = 0;

			uint rayTracingPipelineBindings = 0;
			uint traceRaysCalls = 0;
			uint buildAsCalls = 0;

			//对于Command Buffer
			Nanosecond gpuTime{ 0 };
			Nanosecond cpuTime{ 0 };

			Nanosecond submitingTime{ 0 };
			Nanosecond waitingTime{ 0 };
		};

		struct ResourceStatistics
		{
			uint newGraphicsPipelineCount = 0;
			uint newComputePipelineCount = 0;
			uint newRayTracingPipelineCount = 0;
		};

		struct Statistics
		{
			RenderingStatistics renderer;
			ResourceStatistics resources;

			void Merge(const Statistics&);
		};

		//---------------------------------------------
	//device features & property

		struct DeviceProperties
		{
			bool geometryShader : 1;
			bool tessellationShader : 1;
			bool vertexPipelineStoresAndAtomics : 1;
			bool fragmentStoresAndAtomics : 1;
			bool dedicatedAllocation : 1;
			bool dispatchBase : 1;
			bool imageCubeArray : 1;
			bool array2DCompatible : 1;
			bool blockTexelView : 1;
			bool samplerMirrorClamp : 1;
			bool descriptorIndexing : 1;
			bool drawIndirectCount : 1;
			bool swapchain : 1;
			bool meshShaderNV : 1;
			bool rayTracingNV : 1;

			bool shadingRateImageNV : 1;

			BytesU minSorageBufferOffsetAlignment;
			BytesU minUniformBufferOffsetAlignment;
			uint maxDrawIndirectCount;

			uint maxDrawIndexedIndexValue;
		};

		static constexpr auto	MaxTimeout = Nanosecond{ 60'000'000'000 };

		//接口
	public:

		//初始化//

		GND static FrameGraph CreateFrameGraph(const DeviceInfo_t&);

		GND static const char* GetVersion();

		virtual ~IFrameGraph() {};

		virtual void Deinitialize() = 0;

		virtual bool AddPipelineCompiler(const PipelineCompiler& comp) = 0;

		virtual bool SetShaderDebugCallback(ShaderDebugCallback_t&&) = 0;

		GND virtual DeviceInfo_t GetDeviceInfo() const = 0;

		GND virtual EQueueUsage GetAvilableQueues() const = 0;

		GND virtual DeviceProperties GetDeviceProperties() const = 0;

		//资源管理

		//创建资源 ： Pipeline image buffer
		// 
 

	}
	
	

}