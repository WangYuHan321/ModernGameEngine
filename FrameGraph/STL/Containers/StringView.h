#pragma once

#include <string_view>

namespace FrameGraph
{
	using StringView = std::string_view;
	template <typename T>	using BasicStringView = std::basic_string_view<T>;
}

