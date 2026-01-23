#pragma once

#include <string_view>

namespace FrameGraph
{
	//
	//	Fixed Size Array
	//

	template<typename T, size_t ArraySize>
	struct alignas(std::max(alignof(T), sizeof(void*))) FixedArray
	{
		//types
	public:
		using iterator = T*;
		using const_iterator = const T*;
		using Self = FixedArray<T, ArraySize>;

	private:


	};





}

