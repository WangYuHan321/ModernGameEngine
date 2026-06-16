#pragma once

#include "VBuffer.h"

namespace FrameGraph
{
	//
	// Vulkan Local Buffer
	//
	//	帧内（frame-local）的 buffer 包装：引用一个全局 VBuffer，并在一帧中追踪
	//	对它的读/写访问，从而在执行命令前自动生成所需的内存屏障（pipeline barrier）。
	//	这里采用“整块 buffer”粒度的访问追踪（简化版），覆盖 RAW / WAR / WAW 三类
	//	常见数据冒险。
	//

	class VLocalBuffer final
	{
		//类型
	public:
		// 一次访问的范围信息
		struct BufferAccess
		{
			VkPipelineStageFlags	stages = 0;
			VkAccessFlags			access = 0;
			bool					isReadable = false;
			bool					isWritable = false;
		};

		//变量
	private:
		VBuffer const*	_bufferData = nullptr;	// 引用的全局 buffer（不拥有）

		BufferAccess	_lastAccess;			// 已提交的当前访问作用域
		BufferAccess	_pendingAccess;			// 等待提交的访问

		//方法
	public:
		VLocalBuffer() {}
		VLocalBuffer(VLocalBuffer&&) = delete;
		VLocalBuffer(const VLocalBuffer&) = delete;
		~VLocalBuffer();

		GND bool  Create(VBuffer const* bufferData);
		void  Destroy();

		// 设置初始访问状态（不产生 barrier）
		void  SetInitialState(EResourceState state);

		// 累加一个待提交的访问状态
		void  AddPendingState(EResourceState state);

		// 把 pending 访问与上一次访问做比较，必要时产生 buffer memory barrier，
		// 并把 barrier 追加到 outBarriers，源/目标流水线阶段累加到 srcStage/dstStage。
		void  CommitBarrier(INOUT Array<VkBufferMemoryBarrier>& outBarriers,
							INOUT VkPipelineStageFlags& srcStage,
							INOUT VkPipelineStageFlags& dstStage);

		// 重置帧内追踪状态（一帧开始时调用）
		void  ResetState();

		GND VBuffer const*		ToGlobal()		const { return _bufferData; }
		GND VkBuffer			Handle()		const { return _bufferData ? _bufferData->Handle() : VK_NULL_HANDLE; }
		GND BufferDesc const&	Description()	const { return _bufferData->Description(); }
		GND bool				IsCreated()		const { return _bufferData != nullptr; }
	};

}
