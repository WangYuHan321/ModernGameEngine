#pragma once


#include "StringView.h"

namespace FrameGraph
{
	//
	// Static String
	//

	template <typename CharT, size_t StringSize>
	struct TStaticString
	{
		//type
	public:
		using value_type = CharT;
		using iterator = CharT*;
		using const_iterator = CharT const*;
		using View_t = BasicStringView;
		using Self = TStaticString<CharT, StringSize>;

	private:
		CharT _array[StringSize] = {};
		size_t _length = 0;



	};

	template <size_t StringSize>
	using StaticString = TStaticString< char, StringSize >;
}

