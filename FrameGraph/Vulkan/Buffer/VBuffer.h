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
	class VDevice;

	//
	//Vulkan 立即Buffer
	//
	//	对 VkBuffer 的封装：既支持内部分配并绑定显存（自有），也支持包装外部
	//	已创建的 VkBuffer（不拥有显存，销毁时回调通知）。BufferView 按需惰性
	//	创建并缓存。
	//
	class VBuffer final
	{
		friend class VBufferUnitTest;

		//类型
	private:
		using OnRelease_t = IFrameGraph::OnExternalBufferReleased_t;
		using BufferViewMap_t = HashMap<BufferViewDesc, VkBufferView>;

		//变量
	private:
		VkBuffer _buffer = VK_NULL_HANDLE;
		VkDeviceMemory _memory = VK_NULL_HANDLE;	// 自有显存（外部 buffer 时为 VK_NULL_HANDLE）
		MemoryID _memoryId;
		BufferDesc _desc;

		mutable SharedMutex  _viewMapLock;
		mutable BufferViewMap_t		_viewMap;

		EQueueFamilyMask _queueFamilyMask = Default;
		VkAccessFlags _readAccessMask = 0;

		bool _isExternal = false;

		DebugName_t  _debugName;
		OnRelease_t _onRelease;

		RWDataRaceCheck _drCheck;


	//方法
	public:
		VBuffer() {}
		VBuffer(VBuffer&&) = delete;
		VBuffer(const VBuffer&) = delete;
		~VBuffer();

		// 内部分配显存并创建 buffer
		GND bool  Create(const VDevice& dev,
						  const BufferDesc& desc,
						  EQueueFamilyMask queueFamilyMask,
						  VkMemoryPropertyFlags memFlags,
						  StringView dbgName);

		// 包装外部已创建的 VkBuffer（不拥有显存）
		GND bool  Create(const VDevice& dev,
						  const VulkanBufferDesc& desc,
						  StringView dbgName,
						  OnRelease_t&& onRelease);

		void  Destroy(const VDevice& dev);

		// 惰性创建并缓存 texel buffer view
		GND VkBufferView  GetView(const VDevice& dev, const BufferViewDesc& desc) const;

		GND VulkanBufferDesc  GetApiSpecificDescription() const;

		GND bool				IsCreated()			const { return _buffer != VK_NULL_HANDLE; }
		GND bool				IsExternal()		const { return _isExternal; }

		GND VkBuffer			Handle()			const { return _buffer; }
		GND VkDeviceMemory		GetMemory()			const { return _memory; }
		GND BufferDesc const&	Description()		const { return _desc; }
		GND BytesU				Size()				const { return BytesU{ _desc.size }; }
		GND EQueueFamilyMask	GetQueueFamilyMask() const { return _queueFamilyMask; }
		GND VkAccessFlags		GetAllReadAccessMask() const { return _readAccessMask; }
		GND StringView			GetDebugName()		const { return _debugName; }
	};

}
