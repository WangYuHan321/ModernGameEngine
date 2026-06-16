#pragma once
#include "../Public/FrameGraph.h"
#include "VDevice.h"
#include "VResourceManager.h"
#include "../STL/ThreadSafe/DataRaceCheck.h"

namespace FrameGraph
{
	//
	//   Vulkan FrameGraph
	//
	//	IFrameGraph 接口的 Vulkan 实现。持有 VDevice 与 VResourceManager，
	//	负责初始化 / 反初始化、设备信息与能力查询、以及资源创建的入口。
	//

	class VFrameGraph final : public IFrameGraph
	{
		//类型
	private:
		enum class EState
		{
			Initial,
			Idle,
			Destroyed,
		};

		//变量
	private:
		UniquePtr<VDevice>			_device;
		UniquePtr<VResourceManager>	_resourceMngr;

		Array<PipelineCompiler>		_pplnCompilers;
		ShaderDebugCallback_t		_shaderDebugCallback;

		EState						_state = EState::Initial;

		//方法
	public:
		explicit VFrameGraph(const VulkanDeviceInfo& info);
		~VFrameGraph() override;

		bool  Initialize();

		//---- IFrameGraph ----
		void  Deinitialize() override;
		bool  AddPipelineCompiler(const PipelineCompiler& comp) override;
		bool  SetShaderDebugCallback(ShaderDebugCallback_t&& cb) override;

		GND DeviceInfo_t		GetDeviceInfo()			const override;
		GND EQueueUsage			GetAvilableQueues()		const override;
		GND DeviceProperties	GetDeviceProperties()	const override;

		//---- 内部访问 ----
		GND VDevice const&			GetDevice()				const { return *_device; }
		GND VResourceManager&		GetResourceManager()		  { return *_resourceMngr; }
		GND VResourceManager const&	GetResourceManager()	const { return *_resourceMngr; }

		// 便捷资源接口（转发到 VResourceManager）
		GND RawBufferID		CreateBuffer(const BufferDesc& desc,
										 VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
										 StringView name = {});

		GND VBuffer const*	GetBuffer(RawBufferID id) const;

		bool  ReleaseBuffer(RawBufferID id);
	};

}
