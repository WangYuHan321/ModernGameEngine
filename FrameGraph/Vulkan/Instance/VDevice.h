#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"
#include "../Utils/VEnum.h"
#include "../VCommon.h"
#include "../STL/ThreadSafe/DataRaceCheck.h"

namespace FrameGraph
{
	//
	// Device Queue
	//

	struct VDeviceQueueInfo
	{
		//变量
		
		//当使用 vkQueueSubmit, vkQueueWaitIdle, vkQueueBindSparse, vkQueuePresentKHR
		mutable Mutex guard;
		VkQueue handle = VK_NULL_HANDLE;
		EQueueFamily familyIndex = Default;
		VkQueueFlags familyFlags = Default;
		float priority = 0.0f;
		uint3 minImageTransferGranularity;
		DebugName_t debugName;

		//方法
		VDeviceQueueInfo() {}
		
		VDeviceQueueInfo(VDeviceQueueInfo&& other) :
			handle{ other.handle }, familyIndex{ other.familyIndex }, familyFlags{ other.familyFlags },
			priority{ other.priority }, minImageTransferGranularity{ other.minImageTransferGranularity },
			debugName{ other.debugName }
		{
		}
	};

	//
	//  Vulkan Device
	//
	//	对外部传入的 VkInstance / VkPhysicalDevice / VkDevice 的轻量封装。
	//	负责：缓存设备属性、特性、内存属性；管理逻辑队列；提供内存类型查询与
	//	调试命名等通用工具。本类只“借用”句柄，不负责创建/销毁它们。
	//

	class VDevice final
	{
		friend class VResourceManager;
		friend class VFrameGraph;

		//类型
	public:
		using Queues_t = FixedArray< VDeviceQueueInfo, 16 >;

		// 设备启用的可选特性集合
		struct EnabledFeatures
		{
			bool	imageCubeArray			: 1;
			bool	geometryShader			: 1;
			bool	tessellationShader		: 1;
			bool	samplerAnisotropy		: 1;
			bool	multiDrawIndirect		: 1;
			bool	drawIndirectFirstInstance : 1;
			bool	vertexPipelineStoresAndAtomics : 1;
			bool	fragmentStoresAndAtomics : 1;

			bool	descriptorIndexing		: 1;
			bool	bufferDeviceAddress		: 1;
			bool	timelineSemaphore		: 1;

			bool	meshShaderNV			: 1;
			bool	rayTracingNV			: 1;
			bool	shadingRateImageNV		: 1;

			bool	debugUtils				: 1;
		};

		//变量
	private:
		VkInstance			_instance			= VK_NULL_HANDLE;
		VkPhysicalDevice	_physicalDevice		= VK_NULL_HANDLE;
		VkDevice			_device				= VK_NULL_HANDLE;

		EQueueUsage			_availableQueues	= Default;
		Queues_t			_queues;

		EnabledFeatures		_features			= {};

		VkPhysicalDeviceProperties			_properties			= {};
		VkPhysicalDeviceFeatures			_deviceFeatures		= {};
		VkPhysicalDeviceMemoryProperties	_memoryProperties	= {};

		IFrameGraph::DeviceProperties		_resourceProps		= {};

		const VkAllocationCallbacks*		_allocator			= nullptr;

		PFN_vkSetDebugUtilsObjectNameEXT	_fpSetObjectName	= nullptr;

		DataRaceCheck		_drCheck;

		//方法
	public:
		explicit VDevice(const VulkanDeviceInfo&);
		~VDevice();

		VDevice(const VDevice&) = delete;
		VDevice(VDevice&&) = delete;
		VDevice& operator = (const VDevice&) = delete;
		VDevice& operator = (VDevice&&) = delete;

		GND bool				IsInitialized()			const { return _device != VK_NULL_HANDLE; }

		GND VkInstance			GetVkInstance()			const { return _instance; }
		GND VkPhysicalDevice	GetVkPhysicalDevice()	const { return _physicalDevice; }
		GND VkDevice			GetVkDevice()			const { return _device; }
		GND const VkAllocationCallbacks*	GetAllocator()	const { return _allocator; }

		GND EQueueUsage			GetAvailableQueues()	const { return _availableQueues; }
		GND ArrayView<VDeviceQueueInfo>	GetVkQueues()	const { return _queues; }
		GND VDeviceQueueInfo const*		GetQueue(EQueueType type) const;

		GND const EnabledFeatures&					GetFeatures()				const { return _features; }
		GND const VkPhysicalDeviceProperties&		GetDeviceProperties()		const { return _properties; }
		GND const VkPhysicalDeviceFeatures&			GetDeviceFeatures()			const { return _deviceFeatures; }
		GND const VkPhysicalDeviceLimits&			GetDeviceLimits()			const { return _properties.limits; }
		GND const VkPhysicalDeviceMemoryProperties&	GetMemoryProperties()		const { return _memoryProperties; }
		GND const IFrameGraph::DeviceProperties&	GetResourceProperties()		const { return _resourceProps; }

		// 在 memoryTypeBits 允许的内存类型中，找到同时满足 includeFlags 且不含 excludeFlags 的索引；
		// optFlags 为“最好满足”的标志，找不到时回退到不含 optFlags 的匹配。
		GND bool  GetMemoryTypeIndex(uint memoryTypeBits,
									 VkMemoryPropertyFlags includeFlags,
									 VkMemoryPropertyFlags optFlags,
									 VkMemoryPropertyFlags excludeFlags,
									 OUT uint& memoryTypeIndex) const;

		GND bool  GetMemoryTypeIndex(uint memoryTypeBits,
									 VkMemoryPropertyFlags flags,
									 OUT uint& memoryTypeIndex) const;

		// 给 Vulkan 对象设置调试名（需要 VK_EXT_debug_utils），失败/未启用时返回 false
		bool  SetObjectName(uint64_t id, StringView name, VkObjectType type) const;

	private:
		void  _InitDeviceQueues(const VulkanDeviceInfo&);
		void  _InitDeviceProperties();
		void  _InitFeatures();
		void  _InitResourceProperties();
	};

}
