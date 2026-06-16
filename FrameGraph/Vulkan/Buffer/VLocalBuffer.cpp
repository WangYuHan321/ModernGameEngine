#include "VLocalBuffer.h"

namespace FrameGraph
{

	/*
	=================================================
		destructor
	=================================================
	*/
	VLocalBuffer::~VLocalBuffer()
	{
		ASSERT(_bufferData == nullptr);
	}

	/*
	=================================================
		Create
	=================================================
	*/
	bool  VLocalBuffer::Create(VBuffer const* bufferData)
	{
		CHECK_ERR(_bufferData == nullptr);
		CHECK_ERR(bufferData != nullptr and bufferData->IsCreated());

		_bufferData		= bufferData;
		_lastAccess		= BufferAccess{};
		_pendingAccess	= BufferAccess{};

		return true;
	}

	/*
	=================================================
		Destroy
	=================================================
	*/
	void  VLocalBuffer::Destroy()
	{
		_bufferData		= nullptr;
		_lastAccess		= BufferAccess{};
		_pendingAccess	= BufferAccess{};
	}

	/*
	=================================================
		SetInitialState
	=================================================
	*/
	void  VLocalBuffer::SetInitialState(EResourceState state)
	{
		_lastAccess.stages		= EResourceState_ToStage(state);
		_lastAccess.access		= EResourceState_ToAccess(state);
		_lastAccess.isReadable	= (uint(state) & uint(EResourceState::_Read)) != 0;
		_lastAccess.isWritable	= (uint(state) & uint(EResourceState::_Write)) != 0;

		_pendingAccess = BufferAccess{};
	}

	/*
	=================================================
		AddPendingState
	=================================================
	*/
	void  VLocalBuffer::AddPendingState(EResourceState state)
	{
		_pendingAccess.stages		|= EResourceState_ToStage(state);
		_pendingAccess.access		|= EResourceState_ToAccess(state);
		_pendingAccess.isReadable	|= (uint(state) & uint(EResourceState::_Read)) != 0;
		_pendingAccess.isWritable	|= (uint(state) & uint(EResourceState::_Write)) != 0;
	}

	/*
	=================================================
		CommitBarrier
	---
		需要插入屏障的数据冒险：
		  - 上一次为写，本次为读或写  (RAW / WAW)
		  - 上一次为读，本次为写        (WAR)
		读-读之间无需屏障，只累加读作用域。
	=================================================
	*/
	void  VLocalBuffer::CommitBarrier(INOUT Array<VkBufferMemoryBarrier>& outBarriers,
									  INOUT VkPipelineStageFlags& srcStage,
									  INOUT VkPipelineStageFlags& dstStage)
	{
		if (not (_pendingAccess.isReadable or _pendingAccess.isWritable))
			return;

		const bool needBarrier =
			(_lastAccess.isWritable and (_pendingAccess.isReadable or _pendingAccess.isWritable)) or
			(_lastAccess.isReadable and _pendingAccess.isWritable);

		if (needBarrier and _lastAccess.stages != 0 and _bufferData != nullptr)
		{
			VkBufferMemoryBarrier barrier = {};
			barrier.sType				= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask		= _lastAccess.access;
			barrier.dstAccessMask		= _pendingAccess.access;
			barrier.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer				= _bufferData->Handle();
			barrier.offset				= 0;
			barrier.size				= VK_WHOLE_SIZE;

			outBarriers.push_back(barrier);

			srcStage |= _lastAccess.stages;
			dstStage |= _pendingAccess.stages;
		}

		// 更新“当前访问作用域”
		if (_pendingAccess.isWritable)
		{
			_lastAccess				= _pendingAccess;
		}
		else // 纯读
		{
			if (_lastAccess.isWritable)
			{
				_lastAccess = _pendingAccess;	// 写之后的第一批读，替换作用域
			}
			else
			{
				_lastAccess.stages		|= _pendingAccess.stages;	// 读之后继续读，累加
				_lastAccess.access		|= _pendingAccess.access;
				_lastAccess.isReadable	= true;
			}
		}

		_pendingAccess = BufferAccess{};
	}

	/*
	=================================================
		ResetState
	=================================================
	*/
	void  VLocalBuffer::ResetState()
	{
		_lastAccess		= BufferAccess{};
		_pendingAccess	= BufferAccess{};
	}

}
