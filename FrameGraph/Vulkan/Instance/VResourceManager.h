#pragma once

#include "../Public/FrameGraph.h"
#include "../Public/MemoryDesc.h"
#include "../Shared/LocalResourceID.h"
#include "../Shared/ResourceBase.h"
#include "../Buffer/VBuffer.h"
#include "../Memory/VMemoryObj.h"
#include "../../STL/Containers/ChunkedIndexedPool.h"
#include "../../STL/Memory/UntypedAllocator.h"
#include "../../STL/Common.h"
#include "../Utils/VEnum.h"
#include "../VCommon.h"

namespace FrameGraph
{
	class VDevice;

	//
	// Resource Manager
	//
	// GPU 资源（Buffer / MemoryObj …）的统一生命周期管理。
	//
	// 【ID 模型】RawXxxID = { index, instanceId }
	//   - index：池内槽位，由 ChunkedIndexedPool::Assign 分配/复用
	//   - instanceId：Destroy 后递增（见 ResourceBase），旧句柄自动失效
	//
	// 【池结构】PoolTmpl = ChunkedIndexedPool<ResourceBase<T>, …, Mutex, AtomicPtr>
	//   - 分块存储，槽位地址稳定（ResourceBase 含不可移动成员）
	//   - Assign/Unassign 由池内 Mutex 保护
	//
	// 【创建流程】_Assign → Replace(槽位) → Create → AddRef
	// 【释放流程】ReleaseResource → ReleaseRef 归零 → Destroy → Unassign
	//   注意：仅引用计数归零时才 Unassign，不可每次 Release 都回收槽位
	//

	class VResourceManager final
	{
	public:
		using Index_t = RawBufferID::Index_t;
		using AssignOpGuard_t = Mutex;

		template <typename T, size_t ChunkSize, size_t MaxChunks>
		using PoolTmpl = ChunkedIndexedPool<T, Index_t, ChunkSize, MaxChunks, UntypedAlignedAllocator, AssignOpGuard_t, AtomicPtr>;

		static constexpr uint MaxBuffers = 1u << 10;      // 每块 1024，共 32 块 → 最多 32768 个 Buffer
		static constexpr uint MaxMemoryObjs = 1u << 10;

		using BufferPool_t = PoolTmpl<ResourceBase<VBuffer>, MaxBuffers, 32>;
		using MemoryPool_t = PoolTmpl<ResourceBase<VMemoryObj>, MaxMemoryObjs, 63>;

	private:
		VDevice const& _device;

		BufferPool_t _bufferPool;
		MemoryPool_t _memoryObjPool;

		// GetDescription 在资源无效时返回的空描述，避免悬空引用
		const BufferDesc _dummyBufferDesc;

	public:
		explicit VResourceManager(const VDevice& dev);
		~VResourceManager();

		VResourceManager(const VResourceManager&) = delete;
		VResourceManager& operator = (const VResourceManager&) = delete;

		void Deinitialize();

		GND RawBufferID CreateBuffer(const BufferDesc& desc,
									 const MemoryDesc& mem,
									 EQueueFamilyMask queueFamilyMask,
									 StringView dbgName);

		GND RawBufferID CreateBuffer(const VulkanBufferDesc& desc,
									 IFrameGraph::OnExternalBufferReleased_t&& onRelease,
									 StringView dbgName);

		GND VDevice const& GetDevice() const { return _device; }

		// 通用资源接口（Image / Pipeline 等后续资源类型复用同一套模板）
		template <typename ID>
		bool ReleaseResource(ID id, uint refCount = 1);

		template <typename ID>
		bool AcquireResource(ID id);

		template <typename ID>
		GND bool IsResourceAlive(ID id) const;

		template <typename ID>
		GND auto const* GetResource(ID id, bool incRef = false, bool quiet = false) const;

		template <typename ID>
		GND auto const& GetDescription(ID id) const;

		GND VBuffer const* GetBuffer(RawBufferID id) const;
		GND VMemoryObj const* GetMemory(RawMemoryID id) const;
		bool ReleaseBuffer(RawBufferID id);
		bool ReleaseMemory(RawMemoryID id);

	private:
		bool _CreateMemory(OUT RawMemoryID& id, OUT ResourceBase<VMemoryObj>*& memPtr,
						   const MemoryDesc& desc, StringView dbgName);

		template <typename DataT, size_t CS, size_t MC>
		bool _ReleaseResource(PoolTmpl<DataT, CS, MC>& pool, DataT& data, Index_t index, uint refCount);

		template <typename DataT, size_t CS, size_t MC>
		void _DestroyResourcePool(PoolTmpl<DataT, CS, MC>& pool);

		GND auto& _GetResourcePool(const RawBufferID&) { return _bufferPool; }
		GND auto& _GetResourcePool(const RawMemoryID&) { return _memoryObjPool; }

		// 按 ID 类型分发到对应池（新增资源类型时在此重载）
		template <typename ID>
		GND const auto& _GetResourceCPool(const ID& id) const { return const_cast<VResourceManager*>(this)->_GetResourcePool(id); }

		// 从池取空闲 index，组装 RawXxxID{ index, instanceId }
		template <typename ID>
		GND bool _Assign(OUT ID& id);

		// 创建失败时回收 index（不 Destroy，槽位尚未 Create 成功）
		template <typename ID>
		void _Unassign(ID id);
	};

	/*
	=================================================
		GetResource
	---
		校验 index + instanceId，返回池内资源指针。
		incRef：调用方需要延长生命周期时置 true。
		quiet：true 时不触发 ASSERT（用于探测性查询）。
	=================================================
	*/
	template <typename ID>
	inline auto const* VResourceManager::GetResource(ID id, bool incRef, bool quiet) const
	{
		auto& pool = _GetResourceCPool(id);

		using Result_t = typename std::remove_reference_t<decltype(pool)>::Value_t::Resource_t const*;

		if (id.Index() < pool.size())
		{
			auto& data = pool[id.Index()];

			if (data.IsCreated() and data.GetInstanceID() == id.InstanceID())
			{
				if (incRef)
					data.AddRef();
				return &data.Data();
			}

			ASSERT(quiet or data.IsCreated());
			ASSERT(quiet or data.GetInstanceID() == id.InstanceID());
		}

		(void)quiet;
		if (not quiet)
			ASSERT(false);
		return static_cast<Result_t>(null);
	}

	/*
	=================================================
		GetDescription
	=================================================
	*/
	template <>
	inline auto const& VResourceManager::GetDescription(RawBufferID id) const
	{
		auto* res = GetResource(id);
		return res ? res->Description() : _dummyBufferDesc;
	}

	/*
	=================================================
		IsResourceAlive
	---
		只比对 instanceId，不要求 IsCreated（与原版一致）。
	=================================================
	*/
	template <typename ID>
	inline bool VResourceManager::IsResourceAlive(ID id) const
	{
		ASSERT(id);
		auto& pool = _GetResourceCPool(id);

		return id.Index() < pool.size() and
			   pool[id.Index()].GetInstanceID() == id.InstanceID();
	}

	/*
	=================================================
		AcquireResource
	=================================================
	*/
	template <typename ID>
	inline bool VResourceManager::AcquireResource(ID id)
	{
		ASSERT(id);

		auto& pool = _GetResourcePool(id);

		if (id.Index() < pool.size())
		{
			auto& data = pool[id.Index()];

			if (not data.IsCreated() or data.GetInstanceID() != id.InstanceID())
				return false;

			data.AddRef();
			return true;
		}

		return false;
	}

	/*
	=================================================
		ReleaseResource
	=================================================
	*/
	template <typename ID>
	inline bool VResourceManager::ReleaseResource(ID id, uint refCount)
	{
		ASSERT(id);

		auto& pool = _GetResourcePool(id);

		if (id.Index() >= pool.size())
			return false;

		auto& data = pool[id.Index()];

		if (data.GetInstanceID() != id.InstanceID())
			return false;

		return _ReleaseResource(pool, data, id.Index(), refCount);
	}

	template <typename DataT, size_t CS, size_t MC>
	inline bool VResourceManager::_ReleaseResource(PoolTmpl<DataT, CS, MC>& pool, DataT& data, Index_t index, uint refCount)
	{
		// 仅当引用计数归零且资源仍有效时 Destroy + 回收槽位
		if (data.ReleaseRef(refCount) and data.IsCreated())
		{
			data.Destroy(*this);
			pool.Unassign(index);
			return true;
		}
		return false;
	}

	/*
	=================================================
		_Assign
	---
		从 ChunkedIndexedPool 取空闲 index；
		id 中的 instanceId 取自槽位当前 generation（可能来自上一次 Destroy）。
	=================================================
	*/
	template <typename ID>
	inline bool VResourceManager::_Assign(OUT ID& id)
	{
		auto& pool = _GetResourcePool(id);

		Index_t index = Index_t(UMax);
		CHECK_ERR(pool.Assign(OUT index));

		id = ID(index, pool[index].GetInstanceID());
		return true;
	}

	/*
	=================================================
		_Unassign
	=================================================
	*/
	template <typename ID>
	inline void VResourceManager::_Unassign(ID id)
	{
		ASSERT(id);
		auto& pool = _GetResourcePool(id);
		pool.Unassign(id.Index());
	}

	// 便捷别名（与原版 ReleaseResource / GetResource 等价）
	inline VBuffer const* VResourceManager::GetBuffer(RawBufferID id) const
	{
		return GetResource(id);
	}

	inline VMemoryObj const* VResourceManager::GetMemory(RawMemoryID id) const
	{
		return GetResource(id);
	}

	inline bool VResourceManager::ReleaseBuffer(RawBufferID id)
	{
		return ReleaseResource(id);
	}

	inline bool VResourceManager::ReleaseMemory(RawMemoryID id)
	{
		return ReleaseResource(id);
	}

} // FrameGraph
