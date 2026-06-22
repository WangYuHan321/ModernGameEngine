#pragma once

#include "../../Public/MemoryDesc.h"
#include "../../Public/ResourceEnums.h"
#include "../../STL/Math/Bytes.h"
#include "../VCommon.h"
#include "../STL/ThreadSafe/DataRaceCheck.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	class VDevice;
	class VResourceManager;

	//
	// Vulkan Memory Object
	//
	// 简化版显存对象：Create 记录 MemoryDesc，AllocateForBuffer 时 vkAllocateMemory + Bind。
	// 由 VResourceManager 池化管理；VBuffer 创建时引用并 AddRef。
	//

	class VMemoryObj final
	{
	public:
		// 对外导出的显存信息快照
		struct MemoryInfo
		{
			VkDeviceMemory mem = VK_NULL_HANDLE;
			VkMemoryPropertyFlags flags = 0;
			BytesU offset;
			BytesU size;
			void* mappedPtr = nullptr;
		};

	private:
		// 创建时指定的内存类型与用途（Host / DeviceLocal …）
		MemoryDesc _desc;

		DebugName_t _debugName;

		// Vulkan 显存块
		VkDeviceMemory _memory = VK_NULL_HANDLE;
		VkMemoryPropertyFlags _flags = 0;
		VkDeviceSize _offset = 0;
		VkDeviceSize _size = 0;

		// HostVisible 内存的 CPU 映射指针
		void* _mappedPtr = nullptr;

		bool _allocated = false;

		RWDataRaceCheck _drCheck;

	public:
		VMemoryObj() {}
		VMemoryObj(VMemoryObj&&) = delete;
		VMemoryObj(const VMemoryObj&) = delete;
		~VMemoryObj();

		// EXLOCK(_drCheck)：记录 MemoryDesc，尚未 vkAllocateMemory
		GND bool Create(const MemoryDesc& desc, StringView dbgName);

		// EXLOCK(_drCheck)：释放 VkDeviceMemory，重置状态
		void Destroy(VResourceManager& resMngr);

		// EXLOCK(_drCheck)：为指定 VkBuffer 分配并绑定显存
		GND bool AllocateForBuffer(VDevice& dev, VkBuffer buffer);

		// SHAREDLOCK(_drCheck)：导出当前显存信息
		GND bool GetInfo(OUT MemoryInfo& info) const;

		// SHAREDLOCK(_drCheck)：只读查询内存类型
		GND EMemoryType MemoryType() const { SHAREDLOCK(_drCheck); return _desc.type; }

		// SHAREDLOCK(_drCheck)：只读查询是否已分配
		GND bool IsAllocated() const { SHAREDLOCK(_drCheck); return _allocated; }
	};

} // FrameGraph
