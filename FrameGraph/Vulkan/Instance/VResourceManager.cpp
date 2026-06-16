#include "VResourceManager.h"
#include "VDevice.h"
#include "../Buffer/VBuffer.h"

namespace FrameGraph
{

	/*
	=================================================
		constructor / destructor
	=================================================
	*/
	VResourceManager::VResourceManager(const VDevice& dev) :
		_device{ dev }
	{}

	VResourceManager::~VResourceManager()
	{
		Deinitialize();
	}

	/*
	=================================================
		_AssignBufferSlot
	---
		取一个空闲槽位（优先复用），返回带代数的句柄；调用方需持有 _bufferLock。
	=================================================
	*/
	RawBufferID  VResourceManager::_AssignBufferSlot(OUT VBuffer*& outPtr)
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
			_buffers.push_back(BufferSlot{});
		}

		BufferSlot& slot = _buffers[index];
		if (slot.ptr == nullptr)
			slot.ptr = new VBuffer();

		slot.inUse = true;
		outPtr     = slot.ptr;

		return RawBufferID{ uint16_t(index), uint16_t(slot.generation) };
	}

	/*
	=================================================
		_RecycleSlot
	=================================================
	*/
	void  VResourceManager::_RecycleSlot(uint index)
	{
		BufferSlot& slot = _buffers[index];
		slot.inUse = false;
		++slot.generation;					// 让旧句柄失效
		_freeBuffers.push_back(index);
	}

	/*
	=================================================
		_IsValid
	=================================================
	*/
	bool  VResourceManager::_IsValid(RawBufferID id) const
	{
		if (not id.IsValid())
			return false;

		const uint index = id.Index();
		if (index >= _buffers.size())
			return false;

		const BufferSlot& slot = _buffers[index];
		return slot.inUse and (slot.ptr != nullptr) and (slot.generation == id.InstanceID());
	}

	/*
	=================================================
		CreateBuffer (自有显存)
	=================================================
	*/
	RawBufferID  VResourceManager::CreateBuffer(const BufferDesc& desc, VkMemoryPropertyFlags memFlags,
												EQueueFamilyMask queueFamilyMask, StringView name)
	{
		EXLOCK(_bufferLock);

		VBuffer*	buf = nullptr;
		RawBufferID	id  = _AssignBufferSlot(OUT buf);

		if (not buf->Create(_device, desc, queueFamilyMask, memFlags, name))
		{
			_RecycleSlot(id.Index());
			return RawBufferID{};
		}
		return id;
	}

	/*
	=================================================
		CreateBuffer (外部 buffer)
	=================================================
	*/
	RawBufferID  VResourceManager::CreateBuffer(const VulkanBufferDesc& desc, StringView name,
												IFrameGraph::OnExternalBufferReleased_t&& onRelease)
	{
		EXLOCK(_bufferLock);

		VBuffer*	buf = nullptr;
		RawBufferID	id  = _AssignBufferSlot(OUT buf);

		if (not buf->Create(_device, desc, name, std::move(onRelease)))
		{
			_RecycleSlot(id.Index());
			return RawBufferID{};
		}
		return id;
	}

	/*
	=================================================
		GetBuffer
	=================================================
	*/
	VBuffer const*  VResourceManager::GetBuffer(RawBufferID id) const
	{
		EXLOCK(_bufferLock);

		if (not _IsValid(id))
			return nullptr;

		return _buffers[id.Index()].ptr;
	}

	/*
	=================================================
		ReleaseBuffer
	=================================================
	*/
	bool  VResourceManager::ReleaseBuffer(RawBufferID id)
	{
		EXLOCK(_bufferLock);

		if (not _IsValid(id))
			return false;

		const uint index = id.Index();
		_buffers[index].ptr->Destroy(_device);
		_RecycleSlot(index);
		return true;
	}

	/*
	=================================================
		IsResourceAlive
	=================================================
	*/
	bool  VResourceManager::IsResourceAlive(RawBufferID id) const
	{
		EXLOCK(_bufferLock);
		return _IsValid(id);
	}

	/*
	=================================================
		Deinitialize
	---
		销毁仍在使用的资源并释放池内对象。
	=================================================
	*/
	void  VResourceManager::Deinitialize()
	{
		EXLOCK(_bufferLock);

		for (auto& slot : _buffers)
		{
			if (slot.ptr != nullptr)
			{
				if (slot.inUse)
					slot.ptr->Destroy(_device);

				delete slot.ptr;
				slot.ptr = nullptr;
			}
			slot.inUse = false;
		}

		_buffers.clear();
		_freeBuffers.clear();
	}

}
