#pragma once

#include "../Public/FrameGraph.h"

#include "../Public/MemoryDesc.h"

#include "../Shared/LocalResourceID.h"

#include "../STL/Common.h"

#include "vulkan/vulkan.h"

#include "../Utils/VEnum.h"

#include "../VCommon.h"

#include "../STL/ThreadSafe/DataRaceCheck.h"



namespace FrameGraph

{

	class VDevice;

	class VResourceManager;

	class VMemoryObj;



	//

	// Vulkan Buffer

	//

	//	对 VkBuffer 的封装：内部分配时通过 VMemoryObj 绑定显存；也支持包装外部

	//	已创建的 VkBuffer。BufferView 按需惰性创建并缓存。

	//



	class VBuffer final

	{

		friend class VBufferUnitTest;



	private:

		using OnRelease_t = IFrameGraph::OnExternalBufferReleased_t;

		using BufferViewMap_t = HashMap<BufferViewDesc, VkBufferView>;



	private:

		VkBuffer _buffer = VK_NULL_HANDLE;

		MemoryID _memoryId;

		BufferDesc _desc;



		mutable SharedMutex _viewMapLock;

		mutable BufferViewMap_t _viewMap;



		EQueueFamilyMask _queueFamilyMask = Default;

		VkAccessFlags _readAccessMask = 0;



		bool _isExternal = false;



		DebugName_t _debugName;

		OnRelease_t _onRelease;



		RWDataRaceCheck _drCheck;



	public:

		VBuffer() {}

		VBuffer(VBuffer&&) = delete;

		VBuffer(const VBuffer&) = delete;

		~VBuffer();



		// EXLOCK(_drCheck): 内部分配创建，独占写 _buffer / _desc / _memoryId

		GND bool Create(VResourceManager& resMngr,

						const BufferDesc& desc,

						RawMemoryID memId,

						VMemoryObj& memObj,

						EQueueFamilyMask queueFamilyMask,

						StringView dbgName);



		// EXLOCK(_drCheck): 包装外部 VkBuffer，独占写

		GND bool Create(const VDevice& dev,

						const VulkanBufferDesc& desc,

						StringView dbgName,

						OnRelease_t&& onRelease);



		// EXLOCK(_drCheck): 销毁资源，与 Create / 读接口互斥

		void Destroy(VResourceManager& resMngr);



		// SHAREDLOCK(_viewMapLock) 查缓存；未命中时 EXLOCK(_viewMapLock) 惰性创建

		GND VkBufferView GetView(const VDevice& dev, const BufferViewDesc& desc) const;



		// SHAREDLOCK(_drCheck): 只读快照

		GND VulkanBufferDesc GetApiSpecificDescription() const;



		// SHAREDLOCK(_drCheck): 只读查询 usage

		GND bool IsReadOnly() const;



		// SHAREDLOCK(_drCheck): 只读查询

		GND bool IsCreated() const { SHAREDLOCK(_drCheck); return _buffer != VK_NULL_HANDLE; }



		// SHAREDLOCK(_drCheck): 只读查询

		GND bool IsExternal() const { SHAREDLOCK(_drCheck); return _isExternal; }



		GND bool IsExclusiveSharing() const { return _queueFamilyMask == Default; }



		// SHAREDLOCK(_drCheck): 只读查询 VkBuffer 句柄

		GND VkBuffer Handle() const { SHAREDLOCK(_drCheck); return _buffer; }



		// SHAREDLOCK(_drCheck): 只读查询绑定的显存 ID

		GND RawMemoryID GetMemoryID() const { SHAREDLOCK(_drCheck); return _memoryId.Get(); }



		// SHAREDLOCK(_drCheck): 只读查询描述

		GND BufferDesc const& Description() const { SHAREDLOCK(_drCheck); return _desc; }



		// SHAREDLOCK(_drCheck): 只读查询大小

		GND BytesU Size() const { SHAREDLOCK(_drCheck); return BytesU{ _desc.size }; }



		// SHAREDLOCK(_drCheck): 只读查询队列族掩码

		GND EQueueFamilyMask GetQueueFamilyMask() const { SHAREDLOCK(_drCheck); return _queueFamilyMask; }



		// SHAREDLOCK(_drCheck): 只读查询读访问掩码

		GND VkAccessFlags GetAllReadAccessMask() const { SHAREDLOCK(_drCheck); return _readAccessMask; }



		// SHAREDLOCK(_drCheck): 只读查询调试名

		GND StringView GetDebugName() const { SHAREDLOCK(_drCheck); return _debugName; }



		GND static bool IsSupported(const VDevice& dev, const BufferDesc& desc, EMemoryType memType);



		// SHAREDLOCK(_drCheck): 只读查询 format 支持（依赖 _desc）

		GND bool IsSupported(const VDevice& dev, const BufferViewDesc& desc) const;



	private:

		bool _CreateView(const VDevice& dev, const BufferViewDesc& desc, OUT VkBufferView& view) const;

	};



}


