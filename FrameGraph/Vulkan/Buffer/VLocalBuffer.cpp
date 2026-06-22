#include "VLocalBuffer.h"

namespace FrameGraph
{

	VLocalBuffer::~VLocalBuffer()
	{
		ASSERT(_bufferData == nullptr);
	}

	// 绑定全局 VBuffer，根据 IsReadOnly 设置 _isImmutable
	bool  VLocalBuffer::Create(VBuffer const* bufferData)
	{
		CHECK_ERR(_bufferData == nullptr);
		CHECK_ERR(bufferData != nullptr and bufferData->IsCreated());

		_bufferData = bufferData;
		_isImmutable = bufferData->IsReadOnly();
		_pendingAccesses.clear();
		_accessForWrite.clear();
		_accessForRead.clear();

		return true;
	}

	// 解绑；要求 pending / 历史访问已清空
	void  VLocalBuffer::Destroy()
	{
		ASSERT(_pendingAccesses.empty());
		ASSERT(_accessForWrite.empty());
		ASSERT(_accessForRead.empty());

		_bufferData = nullptr;
		_isImmutable = false;
		_pendingAccesses.clear();
		_accessForWrite.clear();
		_accessForRead.clear();
	}

	VLocalBuffer::BufferRange  VLocalBuffer::_WholeRange(VkDeviceSize size)
	{
		return BufferRange{ 0, size };
	}

	// 手动覆盖不可变标志（pass 录制前设置）
	void  VLocalBuffer::SetInitialState(bool immutable)
	{
		ASSERT(_pendingAccesses.empty());
		ASSERT(_accessForWrite.empty());
		ASSERT(_accessForRead.empty());
		_isImmutable = immutable;
	}

	// 整 buffer 范围的 pending 访问
	void  VLocalBuffer::AddPendingState(EResourceState state)
	{
		if (_bufferData == nullptr)
			return;

		AddPendingState(BufferState{ state, 0, VkDeviceSize(size_t(_bufferData->Size())) });
	}

	// 记录 pending 访问；不可变 buffer 直接跳过
	void  VLocalBuffer::AddPendingState(const BufferState& bs)
	{
		if (_bufferData == nullptr or _isImmutable)
			return;

		BufferAccess pending;
		pending.range = bs.range;
		pending.stages = EResourceState_ToStage(bs.state);
		pending.access = EResourceState_ToAccess(bs.state);
		pending.isReadable = (uint(bs.state) & uint(EResourceState::_Read)) != 0;
		pending.isWritable = (uint(bs.state) & uint(EResourceState::_Write)) != 0;

		_MergePending(_pendingAccesses, pending);
	}

	// 相交 range 合并为一条访问记录
	void  VLocalBuffer::_MergePending(INOUT AccessRecords_t& arr, BufferAccess pending)
	{
		for (auto& rec : arr)
		{
			if (rec.range.IsIntersects(pending.range))
			{
				rec.range.Merge(pending.range);
				rec.stages |= pending.stages;
				rec.access |= pending.access;
				rec.isReadable |= pending.isReadable;
				rec.isWritable |= pending.isWritable;
				return;
			}
		}
		arr.push_back(pending);
	}

	// 判断两段访问在 range 重叠时是否读写冲突
	bool  VLocalBuffer::_RangesConflict(const BufferAccess& prev, const BufferAccess& next)
	{
		if (not prev.range.IsIntersects(next.range))
			return false;

		return (prev.isWritable and (next.isReadable or next.isWritable)) or
			   (prev.isReadable and next.isWritable);
	}

	// 输出一段 range 的 VkBufferMemoryBarrier
	void  VLocalBuffer::_AppendBarrier(INOUT Array<VkBufferMemoryBarrier>& outBarriers,
									   VkBuffer buffer,
									   const BufferAccess& src,
									   const BufferAccess& dst)
	{
		const BufferRange range = src.range.Intersect(dst.range);
		if (range.IsEmpty())
			return;

		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = src.access;
		barrier.dstAccessMask = dst.access;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = buffer;
		barrier.offset = range.begin;
		barrier.size = range.end - range.begin;

		outBarriers.push_back(barrier);
	}

	// pending 与历史读/写对比，生成 barrier 并合并到 _accessForWrite / _accessForRead
	void  VLocalBuffer::CommitBarrier(INOUT Array<VkBufferMemoryBarrier>& outBarriers,
									  INOUT VkPipelineStageFlags& srcStage,
									  INOUT VkPipelineStageFlags& dstStage)
	{
		if (_bufferData == nullptr or _pendingAccesses.empty())
			return;

		for (const BufferAccess& pending : _pendingAccesses)
		{
			for (const BufferAccess& prev : _accessForWrite)
			{
				if (_RangesConflict(prev, pending))
				{
					_AppendBarrier(outBarriers, _bufferData->Handle(), prev, pending);
					srcStage |= prev.stages;
					dstStage |= pending.stages;
				}
			}

			for (const BufferAccess& prev : _accessForRead)
			{
				if (_RangesConflict(prev, pending))
				{
					_AppendBarrier(outBarriers, _bufferData->Handle(), prev, pending);
					srcStage |= prev.stages;
					dstStage |= pending.stages;
				}
			}

			if (pending.isWritable)
				_MergePending(_accessForWrite, pending);
			else if (pending.isReadable)
				_MergePending(_accessForRead, pending);
		}

		_pendingAccesses.clear();
	}

	// 帧/pass 结束：丢弃全部访问历史
	void  VLocalBuffer::ResetState()
	{
		_pendingAccesses.clear();
		_accessForWrite.clear();
		_accessForRead.clear();
	}

} // FrameGraph
