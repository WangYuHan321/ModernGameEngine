#pragma once

#include "../Shared/ResourceDataRange.h"
#include "VBuffer.h"

namespace FrameGraph
{
	//
	// Vulkan Local Buffer
	//
	// 帧内 buffer 状态跟踪：记录 pending 读/写访问，CommitBarrier 时生成
	// VkBufferMemoryBarrier。单线程帧图录制使用，无内部锁。
	//

	class VLocalBuffer final
	{
	public:
		using BufferRange = ResourceDataRange<VkDeviceSize>;

		// 一次 pending 访问：资源状态 + 字节范围
		struct BufferState
		{
			EResourceState state = EResourceState::Unknown;
			BufferRange range;

			BufferState() {}
			BufferState(EResourceState inState, VkDeviceSize begin, VkDeviceSize end) :
				state{ inState }, range{ begin, end } {}
		};

	private:
		// 合并后的访问记录（用于 barrier 生成）
		struct BufferAccess
		{
			BufferRange range;
			VkPipelineStageFlags stages = 0;
			VkAccessFlags access = 0;
			bool isReadable = false;
			bool isWritable = false;
		};

		using AccessRecords_t = Array<BufferAccess>;

	private:
		// 指向全局 VBuffer（不持有所有权，由 VResourceManager 管理）
		VBuffer const* _bufferData = nullptr;

		// true 时跳过 AddPendingState（只读/不可变 buffer）
		bool _isImmutable = false;

		// 本帧尚未 commit 的访问
		AccessRecords_t _pendingAccesses;

		// 已 commit 的历史写/读访问（用于检测 range 冲突）
		AccessRecords_t _accessForWrite;
		AccessRecords_t _accessForRead;

	public:
		VLocalBuffer() {}
		VLocalBuffer(VLocalBuffer&&) = delete;
		VLocalBuffer(const VLocalBuffer&) = delete;
		~VLocalBuffer();

		// 绑定全局 VBuffer，清空访问记录
		GND bool Create(VBuffer const* bufferData);

		// 解绑，要求所有 pending 已 commit 或 reset
		void Destroy();

		// 设置是否不可变（只读 buffer 不再跟踪写）
		void SetInitialState(bool immutable);

		// 记录一段范围的 pending 访问
		void AddPendingState(const BufferState& state);

		// 记录整 buffer 的 pending 访问
		void AddPendingState(EResourceState state);

		// 将 pending 与历史访问对比，输出 buffer memory barrier
		void CommitBarrier(INOUT Array<VkBufferMemoryBarrier>& outBarriers,
						   INOUT VkPipelineStageFlags& srcStage,
						   INOUT VkPipelineStageFlags& dstStage);

		// 清空所有访问记录（帧结束或 pass 重置时用）
		void ResetState();

		// 只读转发：全局 VBuffer
		GND VBuffer const* ToGlobal() const { return _bufferData; }

		// 只读转发：VkBuffer 句柄
		GND VkBuffer Handle() const { return _bufferData ? _bufferData->Handle() : VK_NULL_HANDLE; }

		// 只读转发：BufferDesc
		GND BufferDesc const& Description() const { return _bufferData->Description(); }

		GND bool IsCreated() const { return _bufferData != nullptr; }

	private:
		static BufferRange _WholeRange(VkDeviceSize size);
		static void _MergePending(INOUT AccessRecords_t& arr, BufferAccess pending);
		static bool _RangesConflict(const BufferAccess& prev, const BufferAccess& next);
		static void _AppendBarrier(INOUT Array<VkBufferMemoryBarrier>& outBarriers,
								   VkBuffer buffer,
								   const BufferAccess& src,
								   const BufferAccess& dst);
	};

}
