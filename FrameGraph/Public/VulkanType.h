#pragma once

#include "./Types.h"
#include "../STL/Math/Vec.h"
#include "../STL/Math/Bytes.h"
#include "../STL/Common.h"
#include "../STL/Containers/StringView.h"
#include "../STL/Containers/FixedArray.h"
#include "../STL/Containers/ArrayView.h"

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

	//
	// Vulkan Devie Info
	//

	struct VulkanDeviceInfo
	{
		//类型
		struct QueueInfo
		{
			QueueVk_t            handle = null;
			uint                 familyIndex = {};
			QueueFlagsVk_t       familyFlags = {};
			float				 priority = {};
			StringView           debugName;
		};
		using Queues_t = FixedArray<QueueInfo, 8>;

		//变量
		InstanceVk_t instance = nullptr;
		PhysicalDeviceVk_t physicalDevice = nullptr;
		DeviceVk_t device = nullptr;
		Queues_t queues;

		BytesU maxStagingBufferMemory = ~0_b;
		BytesU StagingBufferMemory = 0_b;
	};

	//
	//Vukan Swapchain Create Info
	//
	struct VulkanSwapchainCreateInfo
	{
		//类型
		using RequiredColorFormat_t = FixedArray< Pair<FormatVk_t, ColorSpaceVk_t>, 4 >;
		using RequiredPresentMode_t = FixedArray<PresentModeVk_t, 4>;

		//变量
		SurfaceVk_t surface = {};
		uint2 surfaceSize;
		uint minImageCount = 2;
		RequiredColorFormat_t formats;
		RequiredPresentMode_t presentModes;
		SurfaceTransformFlagsVk_t preTransform = {};
		CompositeAlphaFlagBitsVk_t compositeAlpha = {};
		ImageUsageVk_t requredUsage = {};
		ImageUsageVk_t optionalUsage = {};
	};

	//
	// Vulkan Image Description
	//
	struct VulkanImageDesc
	{
		ImageVk_t				image = {};
		ImageTypeVk_t			imageType = {};
		ImageFlagsVk_t			flags = {};
		ImageUsageVk_t			usage = {};
		FormatVk_t				format = {};
		ImageLayoutVk_t			currentLayout = {};
		ImageLayoutVk_t			defaultLayout = ImageLayoutVk_t(0x7FFFFFFF);
		SampleCountFlagBitsVk_t	samples = {};
		uint3					dimension;
		uint					arrayLayers = 0;
		uint					maxLevels = 0;
		uint					queueFamily = {};	// queue family that owns image, you must specify this correctly
		// if image created with exclusive sharing mode and you need to
		// keep current content of the image, otherwise keep default value.

		ArrayView<uint>			queueFamilyIndices;		// required if sharing mode is concurent.
	};


	//
	//Vulkan Buffer Descriptor
	//
	struct VulkanBufferDesc
	{
		BufferVk_t buffer = {};
		BufferUsageFlagsVk_t usage = {};
		BytesU size;
		uint queueFamily = {};
		ArrayView<uint> queueFamilyIndices;
	};

	//
	// Vulkan Memory Requirements
	//
	struct VulkanMemRequirements
	{
		uint memTypeBits = 0;
		uint alignment = 0;
	};


	//
	//Vulkan Command Batch
	//
	struct VulkanCommandBatch
	{
		uint queueFamilyIndex = {};
		ArrayView<CommandBufferVk_t> commands;
		ArrayView<SemaphoreVk_t> signalSemaphores;
		ArrayView<Pair<SemaphoreVk_t, PipelineStageFlagsVk_t>> waitSemaphores;
	};

	//
	//Vulkan Context
	//
	struct VulkanContext
	{
		uint queueFamilyIndex = {};
		CommandBufferVk_t commandBuffer = {};
	};

	//
	//Vulkan Draw Context
	//
	struct VulkanDrawContext
	{
		uint queueFamilyIndex = {};
		CommandBufferVk_t comamndBuffer = {};
		RenderPassVk_t renderPass = {};
		uint subpassIndex = {};
	};

}