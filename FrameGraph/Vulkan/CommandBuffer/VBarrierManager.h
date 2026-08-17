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
	//
	// Vulkan Barrier Manager
	//


	class VBarrierManager final
	{

		//type
	private:
		//to do allocator

		using ImageMemoryBarriers_t = Array<VkImageMemoryBarrier>;
		using BufferMemoryBarriers_t = Array<VkBufferMemoryBarrier>;

	private:
		ImageMemoryBarriers_t _imageBarriers;
		BufferMemoryBarriers_t _bufferBarriers;
		VkMemoryBarrier _memoryBarrier;

		VkPipelineStageFlags _srcStageMask = 0;
		VkPipelineStageFlags _dstStageMask = 0;
		VkDependencyFlags	_dependencyFlags = 0;

	public:
		explicit VBarrierManager()
		{
			_imageBarriers.reserve(32);
			_bufferBarriers.reserve(64);
			_memoryBarrier = {};
			_memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		}

		void Commit(const VDevice& dev, VkCommandBuffer cmdBuffer)
		{
			const uint mem_count = !!(_memoryBarrier.srcAccessMask | _memoryBarrier.dstAccessMask);

			if (mem_count or _bufferBarriers.size() or _imageBarriers.size())
			{
				dev.vkCmdPipelineBarrier(cmdBuffer, _srcStageMask, _dstStageMask, _dependencyFlags,
					mem_count, &_memoryBarrier,
					uint(_bufferBarriers.size()),
					_bufferBarriers.data(),
					uint(_imageBarriers.size()),
					_imageBarriers.data());


			}
		}



		void ClearBarriers()
		{
			_imageBarriers.clear();
			_bufferBarriers.clear();
			_memoryBarrier.srcAccessMask = 0;
			_memoryBarrier.dstAccessMask = 0;
			_srcStageMask = 0;
			_dstStageMask = 0;
			_dependencyFlags = 0;
		}

		void AddBufferBarrier(VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			const VkBufferMemoryBarrier& barrier)
		{
			_srcStageMask |= srcStageMask;
			_dstStageMask |= dstStageMask;

			_bufferBarriers.push_back(barrier);
		}

		void AddImageBarrier(VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkDependencyFlags				dependencyFlags,
			const VkImageMemoryBarrier& barrier)
		{
			_srcStageMask |= srcStageMask;
			_dstStageMask |= dstStageMask;
			_dependencyFlags |= dependencyFlags;

			_imageBarriers.push_back(barrier);
		}

		void AddMemoryBarrier(VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			const VkMemoryBarrier& barrier)
		{
			_srcStageMask |= srcStageMask;
			_dstStageMask |= dstStageMask;
			_memoryBarrier.srcAccessMask |= barrier.srcAccessMask;
			_memoryBarrier.dstAccessMask |= barrier.dstAccessMask;
		}
	};


}

