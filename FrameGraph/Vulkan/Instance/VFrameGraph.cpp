#include "VFrameGraph.h"
#include "../Buffer/VBuffer.h"
#include "../../Public/MemoryDesc.h"

namespace FrameGraph
{

	// 构造：创建 VDevice，状态保持 Initial（ResourceManager 在 Initialize 中创建）
	VFrameGraph::VFrameGraph(const VulkanDeviceInfo& info) :
		_device{ std::make_unique<VDevice>(info) },
		_state{ EState::Initial }
	{}

	// 析构：确保 Deinitialize 被调用
	VFrameGraph::~VFrameGraph()
	{
		Deinitialize();
	}

	// 创建 VResourceManager，状态 Initial → Idle
	bool  VFrameGraph::Initialize()
	{
		CHECK_ERR(_device and _device->IsInitialized());

		_resourceMngr = std::make_unique<VResourceManager>(*_device);
		_state = EState::Idle;
		return true;
	}

	// 反初始化：幂等；等待 GPU → 清空资源池 → 释放子对象
	void  VFrameGraph::Deinitialize()
	{
		if (_state == EState::Destroyed)
			return;

		// 等待 GPU 空闲后再销毁资源，避免 vkDestroy 时仍有引用
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

	// 注册 PipelineCompiler；Idle 状态下可调用
	bool  VFrameGraph::AddPipelineCompiler(const PipelineCompiler& comp)
	{
		CHECK_ERR(_state == EState::Idle);
		CHECK_ERR(comp != nullptr);

		for (auto& c : _pplnCompilers)
		{
			if (c == comp)
				return true; // 已存在，跳过
		}

		_pplnCompilers.push_back(comp);
		return true;
	}

	// 保存 Shader 调试回调
	bool  VFrameGraph::SetShaderDebugCallback(ShaderDebugCallback_t&& cb)
	{
		_shaderDebugCallback = std::move(cb);
		return true;
	}

	// 从 VDevice 重建 VulkanDeviceInfo，供上层跨模块传递句柄
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

	// 转发 VDevice：可用队列类型
	EQueueUsage  VFrameGraph::GetAvilableQueues() const
	{
		return _device ? _device->GetAvailableQueues() : EQueueUsage::Unknown;
	}

	// 转发 VDevice：设备特性与限制
	IFrameGraph::DeviceProperties  VFrameGraph::GetDeviceProperties() const
	{
		return _device ? _device->GetResourceProperties() : IFrameGraph::DeviceProperties{};
	}

	// 便捷接口：转发 VResourceManager::CreateBuffer（池级 Mutex + VBuffer EXLOCK）
	RawBufferID  VFrameGraph::CreateBuffer(const BufferDesc& desc, const MemoryDesc& mem, StringView name)
	{
		CHECK_ERR(_resourceMngr);
		return _resourceMngr->CreateBuffer(desc, mem, EQueueFamilyMask::Unknown, name);
	}

	// 便捷接口：转发 VResourceManager::GetBuffer（池级 Mutex + VBuffer SHAREDLOCK）
	VBuffer const*  VFrameGraph::GetBuffer(RawBufferID id) const
	{
		return _resourceMngr ? _resourceMngr->GetBuffer(id) : nullptr;
	}

	// 便捷接口：转发 VResourceManager::ReleaseBuffer
	bool  VFrameGraph::ReleaseBuffer(RawBufferID id)
	{
		CHECK_ERR(_resourceMngr);
		return _resourceMngr->ReleaseBuffer(id);
	}


	// ---- IFrameGraph 静态工厂 ----

	// 工厂：根据 VulkanDeviceInfo 创建 VFrameGraph 并 Initialize
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

	// 返回 FrameGraph 版本字符串
	const char*  IFrameGraph::GetVersion()
	{
		return "ModernGameEngine-FrameGraph 0.1.0 (Vulkan)";
	}

	// 合并两帧统计数据
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

