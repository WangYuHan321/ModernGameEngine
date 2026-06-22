#pragma once

#include "../Public/FrameGraph.h"
#include "../Public/MemoryDesc.h"
#include "../Shared/LocalResourceID.h"
#include "../Shared/ResourceBase.h"
#include "../Buffer/VBuffer.h"
#include "../Memory/VMemoryObj.h"
#include "../STL/Common.h"
#include "../Utils/VEnum.h"
#include "../VCommon.h"

namespace FrameGraph
{
	//
	// Resource Manager
	//
	// 内部资源池：管理 VBuffer / VMemoryObj 的生命周期与 ID 分配。
	// ID = { index, instanceId }，Destroy 后 instanceId 递增，防止悬空句柄。
	// 线程安全：池操作 EXLOCK(_bufferLock / _memoryLock)；
	//           单个资源内部由 VBuffer / VMemoryObj 的 RWDataRaceCheck 保护。
	//

	class VResourceManager final
	{
	private:
		// 关联的 Vulkan 设备（构造时绑定，生命周期长于 ResourceManager）
		const VDevice& _device;

		// Buffer 池：Mutex 保护分配/查询/释放
		mutable Mutex _bufferLock;
		Deque<ResourceBase<VBuffer>> _buffers;  // Deque 避免 vector 扩容移动不可移动的 ResourceBase
		Array<uint> _freeBuffers;               // 已释放槽位索引，供复用

		// Memory 池：与 Buffer 池独立加锁
		mutable Mutex _memoryLock;
		Deque<ResourceBase<VMemoryObj>> _memory;
		Array<uint> _freeMemory;

	public:
		explicit VResourceManager(const VDevice& dev);
		~VResourceManager();

		VResourceManager(const VResourceManager&) = delete;
		VResourceManager& operator = (const VResourceManager&) = delete;

		// 只读查询：关联设备
		GND const VDevice& GetDevice() const { return _device; }

		// EXLOCK(_bufferLock + _memoryLock)：内部分配 Buffer + 绑定 Memory
		GND RawBufferID CreateBuffer(const BufferDesc& desc,
									 const MemoryDesc& mem,
									 EQueueFamilyMask queueFamilyMask,
									 StringView name);

		// EXLOCK(_bufferLock)：包装外部 VkBuffer
		GND RawBufferID CreateBuffer(const VulkanBufferDesc& desc,
									 StringView name,
									 IFrameGraph::OnExternalBufferReleased_t&& onRelease);

		// EXLOCK(_bufferLock)：按 ID 取 VBuffer 指针（只读）
		GND VBuffer const* GetBuffer(RawBufferID id) const;

		// EXLOCK(_memoryLock)：按 ID 取 VMemoryObj 指针（只读）
		GND VMemoryObj const* GetMemory(RawMemoryID id) const;

		// EXLOCK(_bufferLock)：引用计数归零时 Destroy + 回收到 _freeBuffers
		bool ReleaseBuffer(RawBufferID id);

		// EXLOCK(_memoryLock)：引用计数归零时 Destroy + 回收到 _freeMemory
		bool ReleaseMemory(RawMemoryID id);

		// EXLOCK(_bufferLock)：校验 ID 的 index + instanceId 是否仍有效
		GND bool IsResourceAlive(RawBufferID id) const;

		// EXLOCK：销毁所有存活资源并清空池
		void Deinitialize();

	private:
		// 从池中分配/复用 Buffer 槽位，返回带 instanceId 的 RawBufferID
		GND RawBufferID _AssignBuffer(OUT ResourceBase<VBuffer>*& outRes);

		// 从池中分配/复用 Memory 槽位
		GND RawMemoryID _AssignMemory(OUT ResourceBase<VMemoryObj>*& outRes);

		// EXLOCK(_memoryLock)：创建 VMemoryObj 并 AddRef
		bool _CreateMemory(OUT RawMemoryID& memId, OUT ResourceBase<VMemoryObj>*& memObj,
						   const MemoryDesc& mem, StringView dbgName);

		// 将 index 放回空闲列表（调用方已持锁）
		void _UnassignBuffer(uint index);
		void _UnassignMemory(uint index);

		// 校验 ID 合法性（调用方已持锁）
		GND bool _IsValid(RawBufferID id) const;
		GND bool _IsValid(RawMemoryID id) const;

		// 按 ID 取池内 ResourceBase 指针（调用方已持锁）
		GND ResourceBase<VBuffer>* _GetBufferResource(RawBufferID id) const;
		GND ResourceBase<VMemoryObj>* _GetMemoryResource(RawMemoryID id) const;
	};

}
