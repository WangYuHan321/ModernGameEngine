#pragma once

#include <assert.h>
#include <cstdint>
#include <vector>

#include "FrameGraph.h"
#include "Config.h"

#include "../STL/Containers/Ptr.h"
#include "../STL/Containers/StaticString.h"
#include "../STL/Math/Color.h"

namespace FrameGraph
{
	namespace Local
	{
		class IFrameGraphTask {};

		using Task = Ptr<IFrameGraphTask>;


		//
		//   Base Task
		//

		template <typename BaseType>
		struct BaseTask
		{
			//types
			using Tasks_t = FixedArray<Task, GFG_MaxTaskDependencies>;
			using TaskName_t = StaticString;

			Task_t     depends;
			TaskName_t taskName;
			RGBA8u     debugColor;

			BaseTask() {}
			BaseTask(StringView name, RGBA8u color) : taskName{ name }, debugColor{ color } {}

			BaseType& SetName(StringView name) { taskName = name;  return static_cast<BaseType&>(*this); }
			BaseType& SetDebugColor(RGBA8u color) { debugColor = color;  return static_cast<BaseType&>(*this); }

			template <typename Arg0, typename ...Args>
			BaseType& DependsOn(Arg0 task0, Args ...tasks) { if (task0) depends.push_back(task0);  return DependsOn<Args...>(tasks...); }

			template <typename Arg0>
			BaseType& DependsOn(Arg0 task) { if (task) depends.push_back(task);  return static_cast<BaseType&>(*this); }
		};

		//
		//计算着色器调试类型
		//
		struct ComputeShaderDebugInfo
		{
			EShaderDebugMode model = EShaderDebugMode::None;
			uint3            globalID{ ~0u };
		};

		//
		// Ray Tracing Shader Debug Mode
		//
		struct RayTracingShaderDebugMode
		{
			EShaderDebugMode	mode = {};
			uint3				launchID{ ~0u };
		};
	}

	//
	// Image Subresource range
	//

	struct ImageSubresourceRange
	{
		//variables
		EImageAspect aspectMask;
		MipmapLevel mipLevel;
		ImageLayer imageLayer;
		uint layerCount;

		// method
		ImageSubresourceRange() :
			aspectMask{ EImageAspect::Auto }, layerCount(1) {
		}

		explicit
			ImageSubresourceRange(MipmapLevel	mipLevel,
				ImageLayer	baseLayer = {},
				uint			layerCount = 1,
				EImageAspect	aspectMask = EImageAspect::Auto) :
			aspectMask(aspectMask), mipLevel(mipLevel), imageLayer(baseLayer), layerCount(layerCount) {
		}

	};

	//
	// Submit Render Pass
	//

	struct SubmitRenderPass final : Local::BaseTask<SubmitRenderPass>
	{
		//types
		using Images_t = 

	};

}