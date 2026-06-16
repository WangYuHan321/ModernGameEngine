#include "VFrameGraph.h"
#include "../Buffer/VBuffer.h"

namespace FrameGraph
{

	/*
	=================================================
		constructor / destructor
	=================================================
	*/
	VFrameGraph::VFrameGraph(const VulkanDeviceInfo& info) :
		_device{ std::make_unique<VDevice>(info) },
		_state{ EState::Initial }
	{}

	VFrameGraph::~VFrameGraph()
	{
		Deinitialize();
	}

	/*
	=================================================
		Initialize
	=================================================
	*/
	bool  VFrameGraph::Initialize()
	{
		CHECK_ERR(_device and _device->IsInitialized());

		_resourceMngr = std::make_unique<VResourceManager>(*_device);
		_state = EState::Idle;
		return true;
	}

	/*
	=================================================
		Deinitialize
	=================================================
	*/
	void  VFrameGraph::Deinitialize()
	{
		if (_state == EState::Destroyed)
			return;

		// 等待 GPU 空闲后再销毁资源
		if (_device and _device->IsInitialized())
			vkDeviceWaitIdle(_device->GetVkDevice());

		if (_resourceMngr)
			_resourceMngr->Deinitialize();

		_pplnCompilers.clear();
		_shaderDebugCallback = ShaderDebugCallback_t{};

		_resourceMngr.reset();
		_device.reset();

		_state = EState::Destroyed;
	}

	/*
	=================================================
		AddPipelineCompiler
	=================================================
	*/
	bool  VFrameGraph::AddPipelineCompiler(const PipelineCompiler& comp)
	{
		CHECK_ERR(_state == EState::Idle);
		CHECK_ERR(comp != nullptr);

		for (auto& c : _pplnCompilers)
		{
			if (c == comp)
				return true;	// 已存在
		}

		_pplnCompilers.push_back(comp);
		return true;
	}

	/*
	=================================================
		SetShaderDebugCallback
	=================================================
	*/
	bool  VFrameGraph::SetShaderDebugCallback(ShaderDebugCallback_t&& cb)
	{
		_shaderDebugCallback = std::move(cb);
		return true;
	}

	/*
	=================================================
		GetDeviceInfo
	---
		由内部 VDevice 重建出 VulkanDeviceInfo（句柄经不透明类型转换）。
	=================================================
	*/
	IFrameGraph::DeviceInfo_t  VFrameGraph::GetDeviceInfo() const
	{
		if (not _device)
			return DeviceInfo_t{};

		VulkanDeviceInfo info = {};
		info.instance		= reinterpret_cast<InstanceVk_t>(_device->GetVkInstance());
		info.physicalDevice	= reinterpret_cast<PhysicalDeviceVk_t>(_device->GetVkPhysicalDevice());
		info.device			= reinterpret_cast<DeviceVk_t>(_device->GetVkDevice());

		for (auto& q : _device->GetVkQueues())
		{
			if (info.queues.size() >= info.queues.capacity())
				break;

			VulkanDeviceInfo::QueueInfo dst = {};
			dst.handle		= reinterpret_cast<QueueVk_t>(q.handle);
			dst.familyIndex	= uint(q.familyIndex);
			dst.familyFlags	= QueueFlagsVk_t(q.familyFlags);
			dst.priority	= q.priority;
			dst.debugName	= q.debugName;

			info.queues.push_back(dst);
		}

		return DeviceInfo_t{ info };
	}

	/*
	=================================================
		GetAvilableQueues / GetDeviceProperties
	=================================================
	*/
	EQueueUsage  VFrameGraph::GetAvilableQueues() const
	{
		return _device ? _device->GetAvailableQueues() : EQueueUsage::Unknown;
	}

	IFrameGraph::DeviceProperties  VFrameGraph::GetDeviceProperties() const
	{
		return _device ? _device->GetResourceProperties() : IFrameGraph::DeviceProperties{};
	}

	/*
	=================================================
		CreateBuffer / GetBuffer / ReleaseBuffer
	=================================================
	*/
	RawBufferID  VFrameGraph::CreateBuffer(const BufferDesc& desc, VkMemoryPropertyFlags memFlags, StringView name)
	{
		CHECK_ERR(_resourceMngr);
		// 默认使用独占共享模式（EQueueFamilyMask::Unknown）；跨队列族并发访问可在上层指定。
		return _resourceMngr->CreateBuffer(desc, memFlags, EQueueFamilyMask::Unknown, name);
	}

	VBuffer const*  VFrameGraph::GetBuffer(RawBufferID id) const
	{
		return _resourceMngr ? _resourceMngr->GetBuffer(id) : nullptr;
	}

	bool  VFrameGraph::ReleaseBuffer(RawBufferID id)
	{
		CHECK_ERR(_resourceMngr);
		return _resourceMngr->ReleaseBuffer(id);
	}


	//
	// IFrameGraph 静态接口
	//

	/*
	=================================================
		CreateFrameGraph
	=================================================
	*/
	FrameGraph  IFrameGraph::CreateFrameGraph(const DeviceInfo_t& info)
	{
		if (const VulkanDeviceInfo* vkInfo = std::get_if<VulkanDeviceInfo>(&info))
		{
			SharedPtr<VFrameGraph> fg = std::make_shared<VFrameGraph>(*vkInfo);

			if (not fg->Initialize())
				return null;

			return fg;
		}

		FG_LOGE("CreateFrameGraph - unsupported device info type");
		return null;
	}

	/*
	=================================================
		GetVersion
	=================================================
	*/
	const char*  IFrameGraph::GetVersion()
	{
		return "ModernGameEngine-FrameGraph 0.1.0 (Vulkan)";
	}

	/*
	=================================================
		Statistics::Merge
	=================================================
	*/
	void  IFrameGraph::Statistics::Merge(const Statistics& other)
	{
		renderer.descriptorBinds			+= other.renderer.descriptorBinds;
		renderer.pushConstants				+= other.renderer.pushConstants;
		renderer.pipelineBarriers			+= other.renderer.pipelineBarriers;
		renderer.transferOps				+= other.renderer.transferOps;
		renderer.indexBufferBindings		+= other.renderer.indexBufferBindings;
		renderer.vertexBufferBIndings		+= other.renderer.vertexBufferBIndings;
		renderer.drawCalls					+= other.renderer.drawCalls;
		renderer.indirectDrawCalls			+= other.renderer.indirectDrawCalls;
		renderer.vertexCount				+= other.renderer.vertexCount;
		renderer.primitiveCount				+= other.renderer.primitiveCount;
		renderer.graphicsPipelineBindings	+= other.renderer.graphicsPipelineBindings;
		renderer.dynamicStateChanges		+= other.renderer.dynamicStateChanges;
		renderer.dispatchCalls				+= other.renderer.dispatchCalls;
		renderer.computePipelineBinding		+= other.renderer.computePipelineBinding;
		renderer.rayTracingPipelineBindings	+= other.renderer.rayTracingPipelineBindings;
		renderer.traceRaysCalls				+= other.renderer.traceRaysCalls;
		renderer.buildAsCalls				+= other.renderer.buildAsCalls;
		renderer.gpuTime					+= other.renderer.gpuTime;
		renderer.cpuTime					+= other.renderer.cpuTime;
		renderer.submitingTime				+= other.renderer.submitingTime;
		renderer.waitingTime				+= other.renderer.waitingTime;

		resources.newGraphicsPipelineCount		+= other.resources.newGraphicsPipelineCount;
		resources.newComputePipelineCount		+= other.resources.newComputePipelineCount;
		resources.newRayTracingPipelineCount	+= other.resources.newRayTracingPipelineCount;
	}

}
