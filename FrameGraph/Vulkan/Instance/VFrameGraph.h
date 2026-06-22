#pragma once
#include "../Public/FrameGraph.h"
#include "../Public/MemoryDesc.h"
#include "VDevice.h"
#include "VResourceManager.h"

namespace FrameGraph
{
	//
	// Vulkan FrameGraph
	//
	// IFrameGraph 的 Vulkan 实现：
	//   - 对外：IFrameGraph 虚接口（设备查询、编译器注册、生命周期）
	//   - 对内：GetDevice / GetResourceManager + CreateBuffer 等便捷转发
	// 线程安全：本类自身不加锁；并发保护下沉到 VResourceManager（Mutex 池）
	//           以及 VBuffer / VMemoryObj（RWDataRaceCheck）。
	//

	class VFrameGraph final : public IFrameGraph
	{
	private:
		// 生命周期状态机：Initial → Idle → Destroyed
		enum class EState
		{
			Initial,    // 构造完成，VDevice 已创建，ResourceManager 尚未创建
			Idle,       // Initialize 成功，可创建/访问资源
			Destroyed,  // Deinitialize 完成，所有子对象已释放
		};

	private:
		// Vulkan 设备封装：Instance / PhysicalDevice / Device / Queue
		UniquePtr<VDevice> _device;

		// 资源池：Buffer / Memory 的创建、查询、释放（内部 Mutex 保护）
		UniquePtr<VResourceManager> _resourceMngr;

		// 已注册的 SPIR-V → Pipeline 编译器列表（供后续管线创建使用）
		Array<PipelineCompiler> _pplnCompilers;

		// Shader 编译调试输出回调（SetShaderDebugCallback 写入）
		ShaderDebugCallback_t _shaderDebugCallback;

		// 当前生命周期状态，用于 Deinitialize 幂等与 AddPipelineCompiler 前置检查
		EState _state = EState::Initial;

	public:
		// 构造：仅创建 VDevice，不创建 ResourceManager（需调用 Initialize）
		explicit VFrameGraph(const VulkanDeviceInfo& info);

		// 析构：调用 Deinitialize 确保资源释放
		~VFrameGraph() override;

		// 第二阶段初始化：创建 VResourceManager，状态 Initial → Idle
		bool Initialize();

		// ---- IFrameGraph 对外接口 ----

		// 反初始化：等待 GPU 空闲 → 销毁资源池 → 释放 Device，状态 → Destroyed
		void Deinitialize() override;

		// 注册管线编译器；要求 _state == Idle，重复注册同一 comp 直接返回 true
		bool AddPipelineCompiler(const PipelineCompiler& comp) override;

		// 设置 Shader 调试回调，供编译器输出诊断信息
		bool SetShaderDebugCallback(ShaderDebugCallback_t&& cb) override;

		// 导出 Vulkan 句柄与队列信息（供上层重建 VulkanDeviceInfo）
		GND DeviceInfo_t GetDeviceInfo() const override;

		// 查询设备可用队列类型（转发 VDevice）
		GND EQueueUsage GetAvilableQueues() const override;

		// 查询设备特性与限制（转发 VDevice）
		GND DeviceProperties GetDeviceProperties() const override;

		// ---- 内部访问（引擎内部模块使用，非 IFrameGraph 虚接口）----

		// 获取 Vulkan 设备，供 CommandBuffer / Pipeline 等子系统使用
		GND VDevice const& GetDevice() const { return *_device; }

		// 获取可变资源管理器，供内部创建/释放资源
		GND VResourceManager& GetResourceManager() { return *_resourceMngr; }

		// 获取只读资源管理器
		GND VResourceManager const& GetResourceManager() const { return *_resourceMngr; }

		// 便捷接口：CreateBuffer → VResourceManager::CreateBuffer（池锁在 ResourceManager 内）
		GND RawBufferID CreateBuffer(const BufferDesc& desc,
									 const MemoryDesc& mem = MemoryDesc{},
									 StringView name = {});

		// 便捷接口：GetBuffer → VResourceManager::GetBuffer（只读，内部 EXLOCK 池锁）
		GND VBuffer const* GetBuffer(RawBufferID id) const;

		// 便捷接口：ReleaseBuffer → VResourceManager::ReleaseBuffer
		bool ReleaseBuffer(RawBufferID id);
	};

}
