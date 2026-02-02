#pragma once

# include <variant>

namespace FrameGraph
{

	template <typename ...Types>	using Union = std::variant< Types... >;
	using NullUnion = std::monostate;

}

