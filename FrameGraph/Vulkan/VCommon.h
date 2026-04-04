#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include "vulkan/vulkan.h"
#include "../Public/Config.h"
#include "../STL/Containers/StaticString.h"
#include "../STL/Containers/FixedArray.h"

#include "../STL/Containers/Ptr.h"

namespace FrameGraph
{
	using DebugName_t = StaticString<64>;

	using VkDescriptorSets_t = FixedArray<VkDescriptorSet, GFG_MaxDescriptorSets>;


}