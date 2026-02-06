#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include <chrono>
#include <atomic>

#include "../STL/Common.h"
#include "../STL/Containers/Ptr.h"

namespace FrameGraph
{
	using PipelineCompiler = SharedPtr<class IPipelineCompiler>;
	using FrameGraph = SharedPtr<class IFrameGraph>;

	using Task = Ptr< class IFrameGraphTask>;

	using Nanosecond = std::chrono::nanoseconds;
}