#pragma once

#include "./Types.h"
#include "../STL/Containers/StringView.h"

namespace FrameGraph
{

	using InstanceVk_t = struct __FGVkInstance *;
	using PhysicalDeviceVk_t = struct __FGVkPhysicalDevice *;
	using DeviceVk_t = struct __FGDevice *;
	using QueueVk_t = struct __FGVkQueue*;
	using CommandBufferVk_t = struct __FGVkCommandBuffer*;

	enum RenderPassVk_t : uint64_t {};
	enum SurfaceVk_t : uint64_t {};
	enum EventVk_t : uint64_t {};
	enum FenceVk_t : uint64_t {};
	enum BufferVk_t : uint64_t {};
	enum ImageVk_t : uint64_t {};
	enum ShaderModuleVk_t : uint64_t {};
	enum SemaphoreVk_t : uint64_t {};

	enum CompositeAlphaFlagBitsVk_t : uint {};
	enum SurfaceTransformFlagsVk_t : uint {};
	enum PresentModeVk_t : uint {};
	enum QueueFlagsVk_t : uint {};
	enum FormatVk_t : uint {};
	enum ColorSpaceVk_t : uint {};
	enum ImageUsageVk_t : uint {};
	enum BufferUsageFlagsVk_t : uint {};
	enum ImageLayoutVk_t : uint {};
	enum ImageTypeVk_t : uint {};
	enum ImageFlagsVk_t : uint {};
	enum SampleCountFlagBitsVk_t : uint {};
	enum PipelineStageFlagsVk_t : uint {};

	struct VulkanDeviceInfo
	{
		struct QueueInfo
		{
			QueueVk_t            handle = null;
			uint                 familyIndex = {};
			QueueFlagsVk_t       familyFlags = {};
			float				 priority = {};
			StringView           debugName;
		};
	};
}