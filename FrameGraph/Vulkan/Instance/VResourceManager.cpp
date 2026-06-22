#include "VResourceManager.h"
#include "VDevice.h"
#include "../Buffer/VBuffer.h"
#include "../Memory/VMemoryObj.h"

namespace FrameGraph
{

	VResourceManager::VResourceManager(const VDevice& dev) :
		_device{ dev }
	{}

	VResourceManager::~VResourceManager()
	{
		Deinitialize();
	}

	// 从 _freeBuffers 复用或 emplace 新槽位，返回 { index, instanceId }
	RawBufferID  VResourceManager::_AssignBuffer(OUT ResourceBase<VBuffer>*& outRes)
	{
		uint index;

		if (not _freeBuffers.empty())
		{
			index = _freeBuffers.back();
			_freeBuffers.pop_back();
		}
		else
		{
			index = uint(_buffers.size());
			_buffers.emplace_back();
		}

		outRes = &_buffers[index];
		return RawBufferID{ uint16_t(index), outRes->GetInstanceID() };
	}

	// 从 _freeMemory 复用或 emplace 新槽位
	RawMemoryID  VResourceManager::_AssignMemory(OUT ResourceBase<VMemoryObj>*& outRes)
	{
		uint index;

		if (not _freeMemory.empty())
		{
			index = _freeMemory.back();
			_freeMemory.pop_back();
		}
		else
		{
			index = uint(_memory.size());
			_memory.emplace_back();
		}

		outRes = &_memory[index];
		return RawMemoryID{ uint16_t(index), outRes->GetInstanceID() };
	}

	void  VResourceManager::_UnassignBuffer(uint index)
	{
		_freeBuffers.push_back(index);
	}

	void  VResourceManager::_UnassignMemory(uint index)
	{
		_freeMemory.push_back(index);
	}

	// 校验 index 范围 + instanceId 与池内一致
	bool  VResourceManager::_IsValid(RawBufferID id) const
	{
		if (not id.IsValid())
			return false;

		const uint index = id.Index();
		if (index >= _buffers.size())
			return false;

		const ResourceBase<VBuffer>& res = _buffers[index];
		return res.IsCreated() and res.GetInstanceID() == id.InstanceID();
	}

	bool  VResourceManager::_IsValid(RawMemoryID id) const
	{
		if (not id.IsValid())
			return false;

		const uint index = id.Index();
		if (index >= _memory.size())
			return false;

		const ResourceBase<VMemoryObj>& res = _memory[index];
		return res.IsCreated() and res.GetInstanceID() == id.InstanceID();
	}

	ResourceBase<VBuffer>*  VResourceManager::_GetBufferResource(RawBufferID id) const
	{
		if (not _IsValid(id))
			return nullptr;

		return const_cast<ResourceBase<VBuffer>*>(&_buffers[id.Index()]);
	}

	ResourceBase<VMemoryObj>*  VResourceManager::_GetMemoryResource(RawMemoryID id) const
	{
		if (not _IsValid(id))
			return nullptr;

		return const_cast<ResourceBase<VMemoryObj>*>(&_memory[id.Index()]);
	}

	// EXLOCK(_memoryLock)：分配槽位 → VMemoryObj::Create → AddRef
	bool  VResourceManager::_CreateMemory(OUT RawMemoryID& memId, OUT ResourceBase<VMemoryObj>*& memObj,
										  const MemoryDesc& mem, StringView dbgName)
	{
		EXLOCK(_memoryLock);

		memId = _AssignMemory(OUT memObj);
		if (not memObj->Create(mem, dbgName))
		{
			_UnassignMemory(memId.Index());
			memId = RawMemoryID{};
			memObj = nullptr;
			return false;
		}

		memObj->AddRef();
		return true;
	}

	// EXLOCK(_memoryLock) + EXLOCK(_bufferLock)：先建 Memory，再建 Buffer 并绑定
	RawBufferID  VResourceManager::CreateBuffer(const BufferDesc& desc, const MemoryDesc& mem,
												EQueueFamilyMask queueFamilyMask, StringView name)
	{
		RawMemoryID memId;
		ResourceBase<VMemoryObj>* memObj = nullptr;
		CHECK_ERR(_CreateMemory(OUT memId, OUT memObj, mem, name), RawBufferID{});

		EXLOCK(_bufferLock);

		ResourceBase<VBuffer>* bufRes = nullptr;
		RawBufferID id = _AssignBuffer(OUT bufRes);

		if (not bufRes->Create(*this, desc, memId, memObj->Data(), queueFamilyMask, name))
		{
			ReleaseMemory(memId);
			_UnassignBuffer(id.Index());
			return RawBufferID{};
		}

		bufRes->AddRef();
		return id;
	}

	// EXLOCK(_bufferLock)：外部 VkBuffer 包装，不分配 Memory
	RawBufferID  VResourceManager::CreateBuffer(const VulkanBufferDesc& desc, StringView name,
												IFrameGraph::OnExternalBufferReleased_t&& onRelease)
	{
		EXLOCK(_bufferLock);

		ResourceBase<VBuffer>* bufRes = nullptr;
		RawBufferID id = _AssignBuffer(OUT bufRes);

		if (not bufRes->Create(_device, desc, name, std::move(onRelease)))
		{
			_UnassignBuffer(id.Index());
			return RawBufferID{};
		}

		bufRes->AddRef();
		return id;
	}

	// EXLOCK(_bufferLock)：返回池内 VBuffer 指针
	VBuffer const*  VResourceManager::GetBuffer(RawBufferID id) const
	{
		EXLOCK(_bufferLock);

		ResourceBase<VBuffer>* res = _GetBufferResource(id);
		return res ? &res->Data() : nullptr;
	}

	// EXLOCK(_memoryLock)：返回池内 VMemoryObj 指针
	VMemoryObj const*  VResourceManager::GetMemory(RawMemoryID id) const
	{
		EXLOCK(_memoryLock);

		ResourceBase<VMemoryObj>* res = _GetMemoryResource(id);
		return res ? &res->Data() : nullptr;
	}

	// EXLOCK(_bufferLock)：ReleaseRef → 必要时 Destroy → 回收槽位
	bool  VResourceManager::ReleaseBuffer(RawBufferID id)
	{
		EXLOCK(_bufferLock);

		ResourceBase<VBuffer>* res = _GetBufferResource(id);
		if (res == nullptr)
			return false;

		if (res->ReleaseRef(1))
			res->Destroy(*this);

		_UnassignBuffer(id.Index());
		return true;
	}

	// EXLOCK(_memoryLock)：ReleaseRef → 必要时 Destroy → 回收槽位
	bool  VResourceManager::ReleaseMemory(RawMemoryID id)
	{
		EXLOCK(_memoryLock);

		ResourceBase<VMemoryObj>* res = _GetMemoryResource(id);
		if (res == nullptr)
			return false;

		if (res->ReleaseRef(1))
			res->Destroy(*this);

		_UnassignMemory(id.Index());
		return true;
	}

	// EXLOCK(_bufferLock)：仅校验 ID 是否仍指向存活资源
	bool  VResourceManager::IsResourceAlive(RawBufferID id) const
	{
		EXLOCK(_bufferLock);
		return _IsValid(id);
	}

	// EXLOCK：依次 Destroy 所有 Buffer / Memory 并清空池
	void  VResourceManager::Deinitialize()
	{
		{
			EXLOCK(_bufferLock);
			for (auto& res : _buffers)
			{
				if (res.IsCreated())
					res.Destroy(*this);
			}
			_buffers.clear();
			_freeBuffers.clear();
		}

		{
			EXLOCK(_memoryLock);
			for (auto& res : _memory)
			{
				if (res.IsCreated())
					res.Destroy(*this);
			}
			_memory.clear();
			_freeMemory.clear();
		}
	}

} // FrameGraph
