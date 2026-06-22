#include "VDevice.h"

namespace FrameGraph
{

	// 绑定外部 Vulkan 句柄，初始化属性/队列/特性
	VDevice::VDevice(const VulkanDeviceInfo& info) :
		_instance{ reinterpret_cast<VkInstance>(info.instance) },
		_physicalDevice{ reinterpret_cast<VkPhysicalDevice>(info.physicalDevice) },
		_device{ reinterpret_cast<VkDevice>(info.device) }
	{
		CHECK(_instance != VK_NULL_HANDLE);
		CHECK(_physicalDevice != VK_NULL_HANDLE);
		CHECK(_device != VK_NULL_HANDLE);

		_InitDeviceProperties();
		_InitDeviceQueues(info);
		_InitFeatures();
		_InitResourceProperties();

		// 加载调试命名扩展函数（VK_EXT_debug_utils），存在则启用
		_fpSetObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
			vkGetInstanceProcAddr(_instance, "vkSetDebugUtilsObjectNameEXT"));

		_features.debugUtils = (_fpSetObjectName != nullptr);
	}

	// 不销毁外部 Vulkan 对象，仅清空队列列表
	VDevice::~VDevice()
	{
		_queues.clear();
	}

	// 缓存 VkPhysicalDeviceProperties / Features / MemoryProperties
	void VDevice::_InitDeviceProperties()
	{
		vkGetPhysicalDeviceProperties(_physicalDevice, OUT &_properties);
		vkGetPhysicalDeviceFeatures(_physicalDevice, OUT &_deviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(_physicalDevice, OUT &_memoryProperties);
	}

	// 从 VulkanDeviceInfo::queues 填充 _queues，汇总 _availableQueues
	void VDevice::_InitDeviceQueues(const VulkanDeviceInfo& info)
	{
		uint familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, OUT &familyCount, nullptr);

		Array<VkQueueFamilyProperties> familyProps(familyCount);
		if (familyCount > 0)
			vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, OUT &familyCount, OUT familyProps.data());

		uint availableQueues = 0;

		for (auto& srcQueue : info.queues)
		{
			if (_queues.size() >= _queues.capacity())
				break;

			const uint			familyIdx = srcQueue.familyIndex;
			VDeviceQueueInfo&	dstQueue  = _queues.emplace_back();

			dstQueue.handle      = reinterpret_cast<VkQueue>(srcQueue.handle);
			dstQueue.familyIndex = EQueueFamily(familyIdx);
			dstQueue.priority    = srcQueue.priority;
			dstQueue.debugName   = srcQueue.debugName;

			if (familyIdx < familyProps.size())
			{
				const auto& props = familyProps[familyIdx];
				dstQueue.familyFlags = props.queueFlags;
				dstQueue.minImageTransferGranularity = uint3{ props.minImageTransferGranularity.width,
															  props.minImageTransferGranularity.height,
															  props.minImageTransferGranularity.depth };
			}
			else
			{
				dstQueue.familyFlags = VkQueueFlags(srcQueue.familyFlags);
			}

			const VkQueueFlags flags = dstQueue.familyFlags;
			if (flags & VK_QUEUE_GRAPHICS_BIT)
				availableQueues |= uint(EQueueUsage::Graphics);
			else if (flags & VK_QUEUE_COMPUTE_BIT)
				availableQueues |= uint(EQueueUsage::AsyncCompute);
			else if (flags & VK_QUEUE_TRANSFER_BIT)
				availableQueues |= uint(EQueueUsage::AsyncTransfer);
		}

		_availableQueues = EQueueUsage(availableQueues);
	}

	// 从 VkPhysicalDeviceFeatures 解析基础特性（扩展特性默认 false）
	void VDevice::_InitFeatures()
	{
		EnabledFeatures& f = _features;

		f.imageCubeArray				= _deviceFeatures.imageCubeArray;
		f.geometryShader				= _deviceFeatures.geometryShader;
		f.tessellationShader			= _deviceFeatures.tessellationShader;
		f.samplerAnisotropy				= _deviceFeatures.samplerAnisotropy;
		f.multiDrawIndirect				= _deviceFeatures.multiDrawIndirect;
		f.drawIndirectFirstInstance		= _deviceFeatures.drawIndirectFirstInstance;
		f.vertexPipelineStoresAndAtomics = _deviceFeatures.vertexPipelineStoresAndAtomics;
		f.fragmentStoresAndAtomics		= _deviceFeatures.fragmentStoresAndAtomics;

		f.descriptorIndexing			= false;
		f.bufferDeviceAddress			= false;
		f.timelineSemaphore				= false;
		f.meshShaderNV					= false;
		f.rayTracingNV					= false;
		f.shadingRateImageNV			= false;
		f.debugUtils					= false;
	}

	// 填充 IFrameGraph::DeviceProperties 摘要
	void VDevice::_InitResourceProperties()
	{
		const VkPhysicalDeviceLimits&		lim = _properties.limits;
		IFrameGraph::DeviceProperties&		dst = _resourceProps;

		dst.geometryShader					= _deviceFeatures.geometryShader;
		dst.tessellationShader				= _deviceFeatures.tessellationShader;
		dst.vertexPipelineStoresAndAtomics	= _deviceFeatures.vertexPipelineStoresAndAtomics;
		dst.fragmentStoresAndAtomics		= _deviceFeatures.fragmentStoresAndAtomics;
		dst.dedicatedAllocation				= false;
		dst.dispatchBase					= false;
		dst.imageCubeArray					= _deviceFeatures.imageCubeArray;
		dst.array2DCompatible				= false;
		dst.blockTexelView					= false;
		dst.samplerMirrorClamp				= false;
		dst.descriptorIndexing				= _features.descriptorIndexing;
		dst.drawIndirectCount				= _features.multiDrawIndirect;
		dst.swapchain						= true;
		dst.meshShaderNV					= _features.meshShaderNV;
		dst.rayTracingNV					= _features.rayTracingNV;
		dst.shadingRateImageNV				= _features.shadingRateImageNV;

		dst.minSorageBufferOffsetAlignment	= BytesU{ lim.minStorageBufferOffsetAlignment };
		dst.minUniformBufferOffsetAlignment	= BytesU{ lim.minUniformBufferOffsetAlignment };
		dst.maxDrawIndirectCount			= lim.maxDrawIndirectCount;
		dst.maxDrawIndexedIndexValue		= lim.maxDrawIndexedIndexValue;
	}

	// 按 EQueueType 选取最合适的逻辑队列
	VDeviceQueueInfo const*  VDevice::GetQueue(EQueueType type) const
	{
		switch (type)
		{
			case EQueueType::Graphics:
				for (auto& q : _queues)
					if (q.familyFlags & VK_QUEUE_GRAPHICS_BIT)
						return &q;
				break;

			case EQueueType::AsyncCompute:
				for (auto& q : _queues)
					if ((q.familyFlags & VK_QUEUE_COMPUTE_BIT) and not (q.familyFlags & VK_QUEUE_GRAPHICS_BIT))
						return &q;
				for (auto& q : _queues)
					if (q.familyFlags & VK_QUEUE_COMPUTE_BIT)
						return &q;
				break;

			case EQueueType::AsyncTransfer:
				for (auto& q : _queues)
					if ((q.familyFlags & VK_QUEUE_TRANSFER_BIT) and not (q.familyFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)))
						return &q;
				for (auto& q : _queues)
					if (q.familyFlags & VK_QUEUE_TRANSFER_BIT)
						return &q;
				break;

			case EQueueType::_Count:
			case EQueueType::Unknow:
			default:
				break;
		}

		return _queues.empty() ? nullptr : &_queues.front();
	}

	// 两遍扫描：优先 include+opt flags，回退到仅 include flags
	bool  VDevice::GetMemoryTypeIndex(uint memoryTypeBits,
									  VkMemoryPropertyFlags includeFlags,
									  VkMemoryPropertyFlags optFlags,
									  VkMemoryPropertyFlags excludeFlags,
									  OUT uint& memoryTypeIndex) const
	{
		memoryTypeIndex = ~0u;

		const VkMemoryPropertyFlags requiredWithOpt = includeFlags | optFlags;

		// 第一遍：尽量满足 includeFlags + optFlags
		for (uint i = 0; i < _memoryProperties.memoryTypeCount; ++i)
		{
			if ((memoryTypeBits & (1u << i)) == 0)
				continue;

			const VkMemoryPropertyFlags f = _memoryProperties.memoryTypes[i].propertyFlags;

			if ((f & requiredWithOpt) != requiredWithOpt)	continue;
			if ((f & excludeFlags) != 0)					continue;

			memoryTypeIndex = i;
			return true;
		}

		// 第二遍：仅满足 includeFlags
		for (uint i = 0; i < _memoryProperties.memoryTypeCount; ++i)
		{
			if ((memoryTypeBits & (1u << i)) == 0)
				continue;

			const VkMemoryPropertyFlags f = _memoryProperties.memoryTypes[i].propertyFlags;

			if ((f & includeFlags) != includeFlags)	continue;
			if ((f & excludeFlags) != 0)			continue;

			memoryTypeIndex = i;
			return true;
		}

		return false;
	}

	bool  VDevice::GetMemoryTypeIndex(uint memoryTypeBits, VkMemoryPropertyFlags flags, OUT uint& memoryTypeIndex) const
	{
		return GetMemoryTypeIndex(memoryTypeBits, flags, 0, 0, OUT memoryTypeIndex);
	}

	// VK_EXT_debug_utils：给 Buffer / Memory 等对象设调试名
	bool  VDevice::SetObjectName(uint64_t id, StringView name, VkObjectType type) const
	{
		if (not _features.debugUtils or _fpSetObjectName == nullptr or name.empty() or id == 0)
			return false;

		// StringView 不保证以 '\0' 结尾，拷贝一份保证安全
		const String nameStr{ name };

		VkDebugUtilsObjectNameInfoEXT info = {};
		info.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.objectType		= type;
		info.objectHandle	= id;
		info.pObjectName	= nameStr.c_str();

		return _fpSetObjectName(_device, &info) == VK_SUCCESS;
	}

}
