#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include "Types.h"

namespace FrameGraph
{
	//
	// BInding Index
	//
	struct BindingIndex
	{
		using Index_t = uint16_t;

	private:
		Index_t _index1 = ~0u;// 依赖的索引
		Index_t _index2 = ~0u;// resource在Vulkan 中的索引

	public:
		BindingIndex() {}

		explicit BindingIndex(uint perResourceIndex, uint uniqueIndex) : _index1(perResourceIndex), _index2(uniqueIndex) {}

		GND bool		operator == (const BindingIndex& rhs) const { return _index1 == rhs._index1 && _index2 == rhs._index2; }
		GND bool		operator != (const BindingIndex& rhs) const { return not (*this == rhs); }

		GND Index_t		GLBinding()	const { return _index1; }
		GND Index_t		DXBinding()	const { return _index1; }
		GND Index_t		VKBinding()	const { return _index2; }
		GND Index_t		CLBinding()	const { return _index2; }
		GND Index_t		SWBinding()	const { return _index2; }
		GND Index_t		Unique()		const { return _index2; }

	};

}