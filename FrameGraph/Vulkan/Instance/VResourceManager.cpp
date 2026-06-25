#include "VResourceManager.h"
#include "VDevice.h"

namespace FrameGraph
{

	namespace
	{
		// 复用池槽位前先析构旧 ResType 再 placement-new，避免残留状态
		template <typename ResType>
		inline void Replace(INOUT ResourceBase<ResType>& target)
		{
			target.Data().~ResType();
			new (&target.Data()) ResType{};
		}
	}

	VResourceManager::VResourceManager(const VDevice& dev) :
		_device{ dev }
	{}

	VResourceManager::~VResourceManager()
	{
		Deinitialize();
	}

	// _Assign → Replace → Create；失败则 _Unassign 归还 index
	bool  VResourceManager::_CreateMemory(OUT RawMemoryID& id, OUT ResourceBase<VMemoryObj>*& memPtr,
											const MemoryDesc& desc, StringView dbgName)
	{
		CHECK_ERR(_Assign(OUT id));

		auto& data = _GetResourcePool(id)[id.Index()];
		Replace(data);

		if (not data.Create(desc, dbgName))
		{
			_Unassign(id);
			return false;
		}

		memPtr = &data;
		return true;
	}

	// 内部分配 Buffer：先建 MemoryObj，Buffer 持有 mem_id 并 AddRef
	RawBufferID  VResourceManager::CreateBuffer(const BufferDesc& desc, const MemoryDesc& mem,
												 EQueueFamilyMask queueFamilyMask, StringView dbgName)
	{
		RawMemoryID mem_id;
		ResourceBase<VMemoryObj>* mem_obj = null;
		CHECK_ERR(_CreateMemory(OUT mem_id, OUT mem_obj, mem, dbgName), RawBufferID{});

		RawBufferID id;
		CHECK_ERR(_Assign(OUT id), RawBufferID{});

		auto& data = _GetResourcePool(id)[id.Index()];
		Replace(data);

		if (not data.Create(*this, desc, mem_id, mem_obj->Data(), queueFamilyMask, dbgName))
		{
			ReleaseResource(mem_id);
			_Unassign(id);
			return RawBufferID{};
		}

		mem_obj->AddRef();
		data.AddRef();
		return id;
	}

	// 包装外部 VkBuffer，不创建 MemoryObj
	RawBufferID  VResourceManager::CreateBuffer(const VulkanBufferDesc& desc,
												 IFrameGraph::OnExternalBufferReleased_t&& onRelease,
												 StringView dbgName)
	{
		RawBufferID id;
		CHECK_ERR(_Assign(OUT id), RawBufferID{});

		auto& data = _GetResourcePool(id)[id.Index()];
		Replace(data);

		if (not data.Create(_device, desc, dbgName, std::move(onRelease)))
		{
			_Unassign(id);
			return RawBufferID{};
		}

		data.AddRef();
		return id;
	}

	//  shutdown：遍历所有已分配槽位，Destroy 存活资源后释放池内存
	template <typename DataT, size_t CS, size_t MC>
	void  VResourceManager::_DestroyResourcePool(PoolTmpl<DataT, CS, MC>& pool)
	{
		for (size_t i = 0, count = pool.size(); i < count; ++i)
		{
			Index_t id = Index_t(i);
			auto& data = pool[id];

			if (data.IsCreated())
			{
				data.Destroy(*this);
				pool.Unassign(id);
			}
		}
		pool.Release();
	}

	void  VResourceManager::Deinitialize()
	{
		_DestroyResourcePool(_bufferPool);
		_DestroyResourcePool(_memoryObjPool);
	}

} // FrameGraph
