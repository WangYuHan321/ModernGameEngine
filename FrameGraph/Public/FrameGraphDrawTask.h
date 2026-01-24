#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>
#include "./FrameGraph.h"
#include "../STL/Math/Vec.h"
#include "../STL/Containers/StaticString.h"

namespace FrameGraph
{
	namespace Local
	{
		using TaskName_t = StaticString<64>;

		//
		//Base Draw Task
		//
		template <typename TaskType>
		struct BaseDrawTask
		{
			//variables
			TaskName_t taskName;
			RGBAu8     debugColor;

			BaseDrawTask() {};
			BaseDrawTask(StringView name, RGBAu8 color) : taskName{ name }, debugColor{ color } {}

			TaskType& SetName(StringView name) { taskName = name;  return static_cast<TaskType&>(*this); }
			TaskType& SetDebugColor(RGBA8u color) { debugColor = color;  return static_cast<TaskType&>(*this); }
		};

		//
		// Graphics Shader Debug Mode
		//
		struct GraphicsShaderDebugMode
		{
			EShaderDebugMode mode = {};
			EShaderStage stages = {};
			short2       fragCoord{ 0,0 };
		};

		//
		//Render States
		//
		struct VertexBuffer
		{

		};

	}

}