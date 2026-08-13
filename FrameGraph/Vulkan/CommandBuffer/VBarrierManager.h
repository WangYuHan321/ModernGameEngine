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
			_imageBarriers.reserve( 32 );
			_bufferBarriers.reserve( 64 );
			_memoryBarrier = {};
			_memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		}

		void Commit(const VDevice& dev, VkCommandBuffer cmdBuffer)
		{

		}



	}









}

