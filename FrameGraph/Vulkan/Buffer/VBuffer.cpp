#include "VBuffer.h"
#include "../Instance/VDevice.h"
#include "../Instance/VResourceManager.h"
#include "../Memory/VMemoryObj.h"

namespace FrameGraph
{
namespace {

	GND VkAccessFlags  BufferUsageToReadAccess(EBufferUsage usage)
	{
		VkAccessFlags result = 0;

		if (uint(usage & EBufferUsage::TransferSrc))		result |= VK_ACCESS_TRANSFER_READ_BIT;
		if (uint(usage & EBufferUsage::Uniform))			result |= VK_ACCESS_UNIFORM_READ_BIT;
		if (uint(usage & (EBufferUsage::UniformTexel | EBufferUsage::StorageTexel | EBufferUsage::Storage)))
															result |= VK_ACCESS_SHADER_READ_BIT;
		if (uint(usage & EBufferUsage::Index))				result |= VK_ACCESS_INDEX_READ_BIT;
		if (uint(usage & EBufferUsage::Vertex))				result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		if (uint(usage & EBufferUsage::Indirect))			result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

		return result;
	}

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

	VBuffer::~VBuffer()
	{
		ASSERT(_buffer == VK_NULL_HANDLE);
		ASSERT(not _memoryId);
	}

	// EXLOCK(_drCheck): 内部分配创建，独占写 _buffer / _desc / _memoryId
	bool  VBuffer::Create(VResourceManager& resMngr, const BufferDesc& desc, RawMemoryID memId,
						  VMemoryObj& memObj, EQueueFamilyMask queueFamilyMask, StringView dbgName)
	{
		EXLOCK(_drCheck);
		CHECK_ERR(_buffer == VK_NULL_HANDLE);
		CHECK_ERR(not _memoryId);
		CHECK_ERR(desc.size > 0);
		CHECK_ERR(desc.usage != EBufferUsage::Unknown);

		VDevice const& dev = resMngr.GetDevice();
		ASSERT(IsSupported(dev, desc, memObj.MemoryType()));

		_desc = desc;
		_memoryId = MemoryID{ memId };
		_queueFamilyMask = queueFamilyMask;
		_debugName = dbgName;
		_isExternal = false;

		const VkDevice vkDev = dev.GetVkDevice();

		VkBufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = VkDeviceSize(desc.size);
		info.usage = VEnumCast(desc.usage);
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		FixedArray<uint32_t, 8> queueFamilyIndices = {};

		if (queueFamilyMask != Default)
		{
			info.sharingMode = VK_SHARING_MODE_CONCURRENT;
			info.pQueueFamilyIndices = queueFamilyIndices.data();

			for (uint i = 0, mask = 1u; mask <= uint(queueFamilyMask) and info.queueFamilyIndexCount < queueFamilyIndices.size(); ++i, mask = (1u << i))
			{
				if (uint(queueFamilyMask) & mask)
					queueFamilyIndices[info.queueFamilyIndexCount++] = i;
			}
		}

		if (info.queueFamilyIndexCount < 2)
		{
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.pQueueFamilyIndices = nullptr;
			info.queueFamilyIndexCount = 0;
		}

		if (vkCreateBuffer(vkDev, &info, dev.GetAllocator(), OUT &_buffer) != VK_SUCCESS)
		{
			FG_LOGE("VBuffer::Create - vkCreateBuffer failed");
			_memoryId = {};
			return false;
		}

		if (not memObj.AllocateForBuffer(const_cast<VDevice&>(dev), _buffer))
		{
			vkDestroyBuffer(vkDev, _buffer, dev.GetAllocator());
			_buffer = VK_NULL_HANDLE;
			_memoryId = {};
			return false;
		}

		_readAccessMask = BufferUsageToReadAccess(desc.usage);

		if (not dbgName.empty())
			dev.SetObjectName(uint64_t(_buffer), dbgName, VK_OBJECT_TYPE_BUFFER);

		return true;
	}

	// EXLOCK(_drCheck): 包装外部 VkBuffer，独占写
	bool  VBuffer::Create(const VDevice& dev, const VulkanBufferDesc& desc, StringView dbgName, OnRelease_t&& onRelease)
	{
		EXLOCK(_drCheck);
		CHECK_ERR(_buffer == VK_NULL_HANDLE);
		CHECK_ERR(desc.buffer != BufferVk_t(0));

		_buffer = (VkBuffer)(uint64_t(desc.buffer));
		_isExternal = true;

		_desc.size = static_cast<size_t>(uint64_t(desc.size));
		_desc.usage = BufferUsageFromVk(VkBufferUsageFlags(desc.usage));
		_desc.queues = EQueueUsage::Unknown;

		_debugName = dbgName;
		_onRelease = std::move(onRelease);

		_readAccessMask = BufferUsageToReadAccess(_desc.usage);

		if (not dbgName.empty())
			dev.SetObjectName(uint64_t(_buffer), dbgName, VK_OBJECT_TYPE_BUFFER);

		return true;
	}

	// EXLOCK(_drCheck): 销毁资源，与 Create / 读接口互斥
	void  VBuffer::Destroy(VResourceManager& resMngr)
	{
		EXLOCK(_drCheck);

		VDevice const& dev = resMngr.GetDevice();
		const VkDevice vkDev = dev.GetVkDevice();

		// SHAREDLOCK(_viewMapLock): 清理 view 缓存（独立锁，与 _drCheck 分工）
		{
			SHAREDLOCK(_viewMapLock);
			for (auto& it : _viewMap)
			{
				if (it.second != VK_NULL_HANDLE)
					vkDestroyBufferView(vkDev, it.second, dev.GetAllocator());
			}
			_viewMap.clear();
		}

		if (_isExternal)
		{
			if (_onRelease)
				_onRelease(IFrameGraph::ExternalBuffer_t{ BufferVk_t(uint64_t(_buffer)) });
		}
		else if (_buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(vkDev, _buffer, dev.GetAllocator());
		}

		if (_memoryId)
			resMngr.ReleaseMemory(_memoryId.Release());

		_buffer = VK_NULL_HANDLE;
		_memoryId = {};
		_desc = {};
		_queueFamilyMask = Default;
		_readAccessMask = 0;
		_isExternal = false;
		_onRelease = OnRelease_t{};
		_debugName.clear();
	}

	// SHAREDLOCK(_viewMapLock) 查缓存；未命中时 EXLOCK(_viewMapLock) 惰性创建
	VkBufferView  VBuffer::GetView(const VDevice& dev, const BufferViewDesc& descIn) const
	{
		BufferViewDesc desc = descIn;
		desc.Validate(_desc);

		// 快路径：共享读锁查缓存
		{
			SHAREDLOCK(_viewMapLock);
			auto it = _viewMap.find(desc);
			if (it != _viewMap.end())
				return it->second;
		}

		// 慢路径：独占写锁，创建并插入缓存
		EXLOCK(_viewMapLock);

		auto it = _viewMap.find(desc);
		if (it != _viewMap.end())
			return it->second;

		VkBufferView view = VK_NULL_HANDLE;
		if (not _CreateView(dev, desc, OUT view))
			return VK_NULL_HANDLE;

		_viewMap.insert({ desc, view });
		return view;
	}

	bool  VBuffer::_CreateView(const VDevice& dev, const BufferViewDesc& desc, OUT VkBufferView& view) const
	{
		VkBufferViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		info.buffer = _buffer;
		info.format = VEnumCast(desc.format);
		info.offset = VkDeviceSize(desc.offset);
		info.range = (desc.size == ~size_t(0)) ? VK_WHOLE_SIZE : VkDeviceSize(desc.size);

		if (vkCreateBufferView(dev.GetVkDevice(), &info, dev.GetAllocator(), OUT &view) != VK_SUCCESS)
		{
			FG_LOGE("VBuffer::_CreateView - failed to create buffer view");
			return false;
		}
		return true;
	}

	// SHAREDLOCK(_drCheck): 只读快照
	VulkanBufferDesc  VBuffer::GetApiSpecificDescription() const
	{
		SHAREDLOCK(_drCheck);

		VulkanBufferDesc desc = {};
		desc.buffer = BufferVk_t(uint64_t(_buffer));
		desc.usage = BufferUsageFlagsVk_t(VEnumCast(_desc.usage));
		desc.size = BytesU{ _desc.size };
		return desc;
	}

	// SHAREDLOCK(_drCheck): 只读查询 usage
	bool  VBuffer::IsReadOnly() const
	{
		SHAREDLOCK(_drCheck);
		const uint usage = uint(_desc.usage);
		return (usage & uint(EBufferUsage::TransferDst | EBufferUsage::StorageTexel |
							EBufferUsage::Storage | EBufferUsage::RayTracing)) == 0;
	}

	bool  VBuffer::IsSupported(const VDevice& dev, const BufferDesc& desc, EMemoryType memType)
	{
		(void)memType;

		for (uint t = 1; t <= uint(desc.usage); t <<= 1)
		{
			if ((uint(desc.usage) & t) == 0)
				continue;

			const EBufferUsage usage = EBufferUsage(t);
			switch (usage)
			{
			case EBufferUsage::UniformTexel:
			case EBufferUsage::StorageTexel:
			case EBufferUsage::StorageTexelAtomic:
			case EBufferUsage::TransferSrc:
			case EBufferUsage::TransferDst:
			case EBufferUsage::Uniform:
			case EBufferUsage::Storage:
			case EBufferUsage::Index:
			case EBufferUsage::Vertex:
			case EBufferUsage::Indirect:
				break;
			case EBufferUsage::RayTracing:
				if (not dev.GetFeatures().rayTracingNV) return false;
				break;
			case EBufferUsage::VertexPplnStore:
				if (not dev.GetResourceProperties().vertexPipelineStoresAndAtomics) return false;
				break;
			case EBufferUsage::FragmentPplnStore:
				if (not dev.GetResourceProperties().fragmentStoresAndAtomics) return false;
				break;
			default:
				break;
			}
		}
		return true;
	}

	// SHAREDLOCK(_drCheck): 只读查询 format 支持（依赖 _desc）
	bool  VBuffer::IsSupported(const VDevice& dev, const BufferViewDesc& desc) const
	{
		SHAREDLOCK(_drCheck);

		VkFormatProperties props = {};
		vkGetPhysicalDeviceFormatProperties(dev.GetVkPhysicalDevice(), VEnumCast(desc.format), OUT &props);

		const VkFormatFeatureFlags required =
			VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT | VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT;

		return (props.bufferFeatures & required) != 0;
	}

} // FrameGraph
