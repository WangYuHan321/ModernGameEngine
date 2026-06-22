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
	// Vulkan 逻辑队列包装
	//
	// 每个 VkQueue 对应一项；guard 保护 vkQueueSubmit / Present 等队列操作。
	//

	struct VDeviceQueueInfo
	{
		// Mutex：提交/等待/Present 时 EXLOCK(guard)，避免同队列并发调用 Vulkan
		mutable Mutex guard;

		// Vulkan 队列句柄
		VkQueue handle = VK_NULL_HANDLE;

		// 队列族索引（对应 VkQueueFamilyProperties 下标）
		EQueueFamily familyIndex = Default;

		// 队列能力标志（GRAPHICS / COMPUTE / TRANSFER …）
		VkQueueFlags familyFlags = Default;

		// 创建逻辑设备时指定的优先级
		float priority = 0.0f;

		// 图像拷贝/granularity 限制（来自 queue family properties）
		uint3 minImageTransferGranularity;

		// RenderDoc / debug utils 显示用名称
		DebugName_t debugName;

		VDeviceQueueInfo() {}

		VDeviceQueueInfo(VDeviceQueueInfo&& other) :
			handle{ other.handle }, familyIndex{ other.familyIndex }, familyFlags{ other.familyFlags },
			priority{ other.priority }, minImageTransferGranularity{ other.minImageTransferGranularity },
			debugName{ other.debugName }
		{
		}
	};

	//
	// Vulkan Device
	//
	// 封装外部传入的 Instance / PhysicalDevice / Device 句柄（不负责销毁）。
	// 提供队列查询、内存类型选择、设备特性与 debug 命名。
	// 线程安全：构造后只读；DataRaceCheck 预留，当前查询接口无锁。
	//

	class VDevice final
	{
		friend class VResourceManager;
		friend class VFrameGraph;

	public:
		using Queues_t = FixedArray<VDeviceQueueInfo, 16>;

		// 设备已启用/可探测的特性开关（供 VBuffer::IsSupported 等查询）
		struct EnabledFeatures
		{
			bool imageCubeArray : 1;
			bool geometryShader : 1;
			bool tessellationShader : 1;
			bool samplerAnisotropy : 1;
			bool multiDrawIndirect : 1;
			bool drawIndirectFirstInstance : 1;
			bool vertexPipelineStoresAndAtomics : 1;
			bool fragmentStoresAndAtomics : 1;

			bool descriptorIndexing : 1;
			bool bufferDeviceAddress : 1;
			bool timelineSemaphore : 1;

			bool meshShaderNV : 1;
			bool rayTracingNV : 1;
			bool shadingRateImageNV : 1;

			bool debugUtils : 1;
		};

	private:
		// 外部创建的 Vulkan 句柄（VDevice 不销毁）
		VkInstance _instance = VK_NULL_HANDLE;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkDevice _device = VK_NULL_HANDLE;

		// 可用队列类型汇总（Graphics / AsyncCompute / AsyncTransfer）
		EQueueUsage _availableQueues = Default;

		// 逻辑队列列表，与 VulkanDeviceInfo::queues 一一对应
		Queues_t _queues;

		// 解析后的特性开关
		EnabledFeatures _features = {};

		// 物理设备属性 / 特性 / 内存类型表（构造时缓存）
		VkPhysicalDeviceProperties _properties = {};
		VkPhysicalDeviceFeatures _deviceFeatures = {};
		VkPhysicalDeviceMemoryProperties _memoryProperties = {};

		// 面向 IFrameGraph 的设备能力摘要
		IFrameGraph::DeviceProperties _resourceProps = {};

		// 可选：自定义 VkAllocationCallbacks（当前通常为 nullptr）
		const VkAllocationCallbacks* _allocator = nullptr;

		// VK_EXT_debug_utils 函数指针，用于 SetObjectName
		PFN_vkSetDebugUtilsObjectNameEXT _fpSetObjectName = nullptr;

		DataRaceCheck _drCheck;

	public:
		// 构造：绑定外部句柄，初始化属性/队列/特性
		explicit VDevice(const VulkanDeviceInfo&);

		// 析构：仅清空内部状态，不销毁 Vulkan 对象
		~VDevice();

		VDevice(const VDevice&) = delete;
		VDevice(VDevice&&) = delete;
		VDevice& operator = (const VDevice&) = delete;
		VDevice& operator = (VDevice&&) = delete;

		// 只读查询：Device 句柄是否有效
		GND bool IsInitialized() const { return _device != VK_NULL_HANDLE; }

		// 只读查询：Vulkan 核心句柄
		GND VkInstance GetVkInstance() const { return _instance; }
		GND VkPhysicalDevice GetVkPhysicalDevice() const { return _physicalDevice; }
		GND VkDevice GetVkDevice() const { return _device; }
		GND const VkAllocationCallbacks* GetAllocator() const { return _allocator; }

		// 只读查询：队列信息
		GND EQueueUsage GetAvailableQueues() const { return _availableQueues; }
		GND ArrayView<VDeviceQueueInfo> GetVkQueues() const { return _queues; }

		// 按用途选取最合适的队列（Async 优先专用队列）
		GND VDeviceQueueInfo const* GetQueue(EQueueType type) const;

		// 只读查询：特性与限制
		GND const EnabledFeatures& GetFeatures() const { return _features; }
		GND const VkPhysicalDeviceProperties& GetDeviceProperties() const { return _properties; }
		GND const VkPhysicalDeviceFeatures& GetDeviceFeatures() const { return _deviceFeatures; }
		GND const VkPhysicalDeviceLimits& GetDeviceLimits() const { return _properties.limits; }
		GND const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return _memoryProperties; }
		GND const IFrameGraph::DeviceProperties& GetResourceProperties() const { return _resourceProps; }

		// 在 memoryTypeBits 中查找满足 flags 的内存类型索引（VMemoryObj 分配时用）
		GND bool GetMemoryTypeIndex(uint memoryTypeBits,
									VkMemoryPropertyFlags includeFlags,
									VkMemoryPropertyFlags optFlags,
									VkMemoryPropertyFlags excludeFlags,
									OUT uint& memoryTypeIndex) const;

		GND bool GetMemoryTypeIndex(uint memoryTypeBits,
									VkMemoryPropertyFlags flags,
									OUT uint& memoryTypeIndex) const;

		// 给 Vulkan 对象设置调试名（需 VK_EXT_debug_utils）
		bool SetObjectName(uint64_t id, StringView name, VkObjectType type) const;

	private:
		void _InitDeviceQueues(const VulkanDeviceInfo&);
		void _InitDeviceProperties();
		void _InitFeatures();
		void _InitResourceProperties();
	};

}
