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
	class VBuffer;

	//
	// Resource Manager
	//
	//	负责 GPU 资源（当前实现：buffer）的生命周期管理。内部使用带“代数(generation)”
	//	校验的索引池：句柄 = {下标, 代数}，释放后代数自增，从而让旧句柄自动失效，避免
	//	悬垂访问。资源对象以指针持有，地址稳定（VBuffer 含 mutex 不可移动）。
	//

	class VResourceManager final
	{
		//类型
	private:
		struct BufferSlot
		{
			VBuffer*	ptr			= nullptr;
			uint16_t	generation	= 0;
			bool		inUse		= false;
		};

		//变量
	private:
		const VDevice&		_device;

		mutable Mutex		_bufferLock;
		Array<BufferSlot>	_buffers;
		Array<uint>			_freeBuffers;

		//方法
	public:
		explicit VResourceManager(const VDevice& dev);
		~VResourceManager();

		VResourceManager(const VResourceManager&) = delete;
		VResourceManager& operator = (const VResourceManager&) = delete;

		GND const VDevice&  GetDevice() const { return _device; }

		// 创建自有显存的 buffer
		GND RawBufferID  CreateBuffer(const BufferDesc& desc,
									  VkMemoryPropertyFlags memFlags,
									  EQueueFamilyMask queueFamilyMask,
									  StringView name);

		// 注册外部 buffer（不拥有显存，释放时回调）
		GND RawBufferID  CreateBuffer(const VulkanBufferDesc& desc,
									  StringView name,
									  IFrameGraph::OnExternalBufferReleased_t&& onRelease);

		GND VBuffer const*  GetBuffer(RawBufferID id) const;

		bool  ReleaseBuffer(RawBufferID id);

		GND bool  IsResourceAlive(RawBufferID id) const;

		void  Deinitialize();

	private:
		RawBufferID  _AssignBufferSlot(OUT VBuffer*& outPtr);
		bool  _IsValid(RawBufferID id) const;
		void  _RecycleSlot(uint index);
	};

}
