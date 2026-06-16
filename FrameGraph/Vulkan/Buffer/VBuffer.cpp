#include "VBuffer.h"
#include "../Instance/VDevice.h"

namespace FrameGraph
{
namespace {

	//	由 buffer 用途推导“只读访问掩码”，用于 barrier 合批优化。
	GND VkAccessFlags  BufferUsageToReadAccess(EBufferUsage usage)
	{
		VkAccessFlags result = 0;

		if (uint(usage & EBufferUsage::TransferSrc))	result |= VK_ACCESS_TRANSFER_READ_BIT;
		if (uint(usage & EBufferUsage::Uniform))		result |= VK_ACCESS_UNIFORM_READ_BIT;
		if (uint(usage & (EBufferUsage::UniformTexel | EBufferUsage::StorageTexel | EBufferUsage::Storage)))
														result |= VK_ACCESS_SHADER_READ_BIT;
		if (uint(usage & EBufferUsage::Index))			result |= VK_ACCESS_INDEX_READ_BIT;
		if (uint(usage & EBufferUsage::Vertex))			result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		if (uint(usage & EBufferUsage::Indirect))		result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

		return result;
	}

	//	VkBufferUsageFlags -> EBufferUsage（包装外部 buffer 时用）
	GND EBufferUsage  BufferUsageFromVk(VkBufferUsageFlags usage)
	{
		EBufferUsage result = EBufferUsage::Unknown;

		if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)			result |= EBufferUsage::TransferSrc;
		if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)			result |= EBufferUsage::TransferDst;
		if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)	result |= EBufferUsage::UniformTexel;
		if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)	result |= EBufferUsage::StorageTexel;
		if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)			result |= EBufferUsage::Uniform;
		if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)			result |= EBufferUsage::Storage;
		if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)			result |= EBufferUsage::Index;
		if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)			result |= EBufferUsage::Vertex;
		if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)		result |= EBufferUsage::Indirect;

		return result;
	}

} // namespace


	/*
	=================================================
		destructor
	=================================================
	*/
	VBuffer::~VBuffer()
	{
		// 句柄应在持有 VDevice 的 Destroy() 中显式释放；此处仅做断言提醒。
		ASSERT(_buffer == VK_NULL_HANDLE);
		ASSERT(_memory == VK_NULL_HANDLE);
	}

	/*
	=================================================
		Create (内部分配显存)
	=================================================
	*/
	bool  VBuffer::Create(const VDevice& dev, const BufferDesc& desc, EQueueFamilyMask queueFamilyMask,
						  VkMemoryPropertyFlags memFlags, StringView dbgName)
	{
		CHECK_ERR(_buffer == VK_NULL_HANDLE);
		CHECK_ERR(desc.size > 0);

		const VkDevice					vkDev = dev.GetVkDevice();
		const VkAllocationCallbacks*	alloc = dev.GetAllocator();

		_desc            = desc;
		_queueFamilyMask = queueFamilyMask;
		_debugName       = dbgName;
		_isExternal      = false;

		// 1. 创建 VkBuffer（独占共享模式）
		VkBufferCreateInfo info = {};
		info.sType			= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.flags			= 0;
		info.size			= VkDeviceSize(desc.size);
		info.usage			= VEnumCast(desc.usage);
		info.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;

		CHECK_ERR(vkCreateBuffer(vkDev, &info, alloc, OUT &_buffer) == VK_SUCCESS);

		// 2. 查询显存需求并选择内存类型
		VkMemoryRequirements memReq = {};
		vkGetBufferMemoryRequirements(vkDev, _buffer, OUT &memReq);

		uint memTypeIndex = ~0u;
		if (not dev.GetMemoryTypeIndex(memReq.memoryTypeBits, memFlags, OUT memTypeIndex))
		{
			FG_LOGE("VBuffer::Create - no suitable memory type");
			Destroy(dev);
			return false;
		}

		// 3. 分配并绑定
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize	= memReq.size;
		allocInfo.memoryTypeIndex	= memTypeIndex;

		if (vkAllocateMemory(vkDev, &allocInfo, alloc, OUT &_memory) != VK_SUCCESS)
		{
			FG_LOGE("VBuffer::Create - failed to allocate memory");
			Destroy(dev);
			return false;
		}

		if (vkBindBufferMemory(vkDev, _buffer, _memory, 0) != VK_SUCCESS)
		{
			FG_LOGE("VBuffer::Create - failed to bind memory");
			Destroy(dev);
			return false;
		}

		_readAccessMask = BufferUsageToReadAccess(desc.usage);

		if (not dbgName.empty())
			dev.SetObjectName(uint64_t(_buffer), dbgName, VK_OBJECT_TYPE_BUFFER);

		return true;
	}

	/*
	=================================================
		Create (包装外部 buffer)
	=================================================
	*/
	bool  VBuffer::Create(const VDevice& dev, const VulkanBufferDesc& desc, StringView dbgName, OnRelease_t&& onRelease)
	{
		CHECK_ERR(_buffer == VK_NULL_HANDLE);
		CHECK_ERR(desc.buffer != BufferVk_t(0));

		_buffer      = (VkBuffer)(uint64_t(desc.buffer));
		_memory      = VK_NULL_HANDLE;
		_isExternal  = true;

		_desc.size   = static_cast<size_t>(uint64_t(desc.size));
		_desc.usage  = BufferUsageFromVk(VkBufferUsageFlags(desc.usage));
		_desc.queues = EQueueUsage::Unknown;

		_debugName   = dbgName;
		_onRelease   = std::move(onRelease);

		_readAccessMask = BufferUsageToReadAccess(_desc.usage);

		if (not dbgName.empty())
			dev.SetObjectName(uint64_t(_buffer), dbgName, VK_OBJECT_TYPE_BUFFER);

		return true;
	}

	/*
	=================================================
		Destroy
	=================================================
	*/
	void  VBuffer::Destroy(const VDevice& dev)
	{
		const VkDevice					vkDev = dev.GetVkDevice();
		const VkAllocationCallbacks*	alloc = dev.GetAllocator();

		// 销毁所有缓存的 buffer view
		{
			EXLOCK(_viewMapLock);
			for (auto& it : _viewMap)
			{
				if (it.second != VK_NULL_HANDLE)
					vkDestroyBufferView(vkDev, it.second, alloc);
			}
			_viewMap.clear();
		}

		if (_isExternal)
		{
			// 外部 buffer：仅触发回调，不销毁句柄/显存
			if (_onRelease)
				_onRelease(IFrameGraph::ExternalBuffer_t{ BufferVk_t(uint64_t(_buffer)) });
		}
		else
		{
			if (_buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(vkDev, _buffer, alloc);
			if (_memory != VK_NULL_HANDLE)
				vkFreeMemory(vkDev, _memory, alloc);
		}

		_buffer         = VK_NULL_HANDLE;
		_memory         = VK_NULL_HANDLE;
		_isExternal     = false;
		_readAccessMask = 0;
		_onRelease      = OnRelease_t{};
		_debugName.clear();
	}

	/*
	=================================================
		GetView
	---
		先以共享锁查缓存，未命中再以独占锁创建（double-check）。
	=================================================
	*/
	VkBufferView  VBuffer::GetView(const VDevice& dev, const BufferViewDesc& descIn) const
	{
		BufferViewDesc desc = descIn;
		desc.Validate(_desc);

		// 1. 共享锁查缓存
		{
			SHAREDLOCK(_viewMapLock);
			auto it = _viewMap.find(desc);
			if (it != _viewMap.end())
				return it->second;
		}

		// 2. 独占锁创建
		EXLOCK(_viewMapLock);

		auto it = _viewMap.find(desc);
		if (it != _viewMap.end())
			return it->second;

		VkBufferViewCreateInfo info = {};
		info.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		info.buffer = _buffer;
		info.format = VEnumCast(desc.format);
		info.offset = VkDeviceSize(desc.offset);
		info.range  = (desc.size == 0) ? VK_WHOLE_SIZE : VkDeviceSize(desc.size);

		VkBufferView view = VK_NULL_HANDLE;
		if (vkCreateBufferView(dev.GetVkDevice(), &info, dev.GetAllocator(), OUT &view) != VK_SUCCESS)
		{
			FG_LOGE("VBuffer::GetView - failed to create buffer view");
			return VK_NULL_HANDLE;
		}

		_viewMap.insert({ desc, view });
		return view;
	}

	/*
	=================================================
		GetApiSpecificDescription
	=================================================
	*/
	VulkanBufferDesc  VBuffer::GetApiSpecificDescription() const
	{
		VulkanBufferDesc desc = {};
		desc.buffer = BufferVk_t(uint64_t(_buffer));
		desc.usage  = BufferUsageFlagsVk_t(VEnumCast(_desc.usage));
		desc.size   = BytesU{ _desc.size };
		return desc;
	}

}
