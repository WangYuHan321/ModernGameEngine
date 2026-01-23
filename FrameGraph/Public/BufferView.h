#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

namespace FrameGraph
{
	//
	//   FrameGraph 接口
	//

	struct BufferView
	{
		//types
	public:

		using T = uint8_t;
		using value_type = T;

		struct Iterator
		{
			friend struct BufferView;


		};

	};

}