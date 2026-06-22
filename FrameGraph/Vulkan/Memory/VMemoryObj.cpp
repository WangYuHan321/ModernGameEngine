#include "VMemoryObj.h"
#include "../Instance/VDevice.h"
#include "../Instance/VResourceManager.h"
#include "../Utils/VEnum.h"

namespace FrameGraph
{

	VMemoryObj::~VMemoryObj()
	{
		ASSERT(not _allocated);
		ASSERT(_memory == VK_NULL_HANDLE);
	}

	// EXLOCK(_drCheck)：记录 MemoryDesc，尚未分配 VkDeviceMemory
	bool  VMemoryObj::Create(const MemoryDesc& desc, StringView dbgName)
	{
		EXLOCK(_drCheck);
		CHECK_ERR(not _allocated);

		_desc = desc;
		_debugName = dbgName;
		return true;
	}

	// EXLOCK(_drCheck)：Unmap + FreeMemory，重置全部字段
	void  VMemoryObj::Destroy(VResourceManager& resMngr)
	{
		EXLOCK(_drCheck);

		if (_memory != VK_NULL_HANDLE)
		{
			VDevice const& dev = resMngr.GetDevice();
			const VkDevice vkDev = dev.GetVkDevice();

			if (_mappedPtr != nullptr)
			{
				vkUnmapMemory(vkDev, _memory);
				_mappedPtr = nullptr;
			}

			vkFreeMemory(vkDev, _memory, dev.GetAllocator());
			_memory = VK_NULL_HANDLE;
		}

		_allocated = false;
		_flags = 0;
		_offset = 0;
		_size = 0;
		_desc = {};
		_debugName.clear();
	}

	// EXLOCK(_drCheck)：vkAllocateMemory + vkBindBufferMemory，HostVisible 时 Map
	bool  VMemoryObj::AllocateForBuffer(VDevice& dev, VkBuffer buffer)
	{
		EXLOCK(_drCheck);
		CHECK_ERR(not _allocated);
		CHECK_ERR(buffer != VK_NULL_HANDLE);

		const VkDevice vkDev = dev.GetVkDevice();

		VkMemoryRequirements memReq = {};
		vkGetBufferMemoryRequirements(vkDev, buffer, OUT &memReq);

		const VkMemoryPropertyFlags memFlags = VMemoryTypeToFlags(_desc.type);

		uint memTypeIndex = ~0u;
		if (not dev.GetMemoryTypeIndex(memReq.memoryTypeBits, memFlags, OUT memTypeIndex))
		{
			FG_LOGE("VMemoryObj::AllocateForBuffer - no suitable memory type");
			return false;
		}

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = memTypeIndex;

		if (vkAllocateMemory(vkDev, &allocInfo, dev.GetAllocator(), OUT &_memory) != VK_SUCCESS)
		{
			FG_LOGE("VMemoryObj::AllocateForBuffer - vkAllocateMemory failed");
			return false;
		}

		if (vkBindBufferMemory(vkDev, buffer, _memory, 0) != VK_SUCCESS)
		{
			FG_LOGE("VMemoryObj::AllocateForBuffer - vkBindBufferMemory failed");
			vkFreeMemory(vkDev, _memory, dev.GetAllocator());
			_memory = VK_NULL_HANDLE;
			return false;
		}

		_flags = memFlags;
		_offset = 0;
		_size = memReq.size;
		_allocated = true;

		if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
		{
			if (vkMapMemory(vkDev, _memory, 0, memReq.size, 0, OUT &_mappedPtr) != VK_SUCCESS)
				_mappedPtr = nullptr;
		}

		if (not _debugName.empty())
			dev.SetObjectName(uint64_t(_memory), _debugName, VK_OBJECT_TYPE_DEVICE_MEMORY);

		return true;
	}

	// SHAREDLOCK(_drCheck)：导出显存句柄、flags、映射指针等
	bool  VMemoryObj::GetInfo(OUT MemoryInfo& info) const
	{
		SHAREDLOCK(_drCheck);
		CHECK_ERR(_allocated);

		info.mem = _memory;
		info.flags = _flags;
		info.offset = BytesU{ size_t(_offset) };
		info.size = BytesU{ size_t(_size) };
		info.mappedPtr = _mappedPtr;
		return true;
	}

} // FrameGraph

