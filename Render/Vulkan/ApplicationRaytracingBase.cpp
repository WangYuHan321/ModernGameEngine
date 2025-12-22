#include "ApplicationRaytracingBase.h"

void ApplicationRaytracingBase::SetupRenderPass()
{
	//移除之前的renderPass 为了支持drawUI
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	VkAttachmentLoadOp colorLoadOp{ VK_ATTACHMENT_LOAD_OP_LOAD };//“读旧值，在其基础上继续画” 性能开销最大
	VkImageLayout colorInitialLayout{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

	if (rayQueryOnly)
	{
		colorLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	std::array<VkAttachmentDescription, 2> attachments{
		VkAttachmentDescription{
			.format = m_swapChain.colorFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = colorLoadOp,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = colorInitialLayout,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},
				VkAttachmentDescription{
			.format = m_depthFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}
	};

	VkAttachmentReference colorReference{ .attachment = 0,.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference{ .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorReference,
		.pDepthStencilAttachment = &depthReference,
	};

	std::array<VkSubpassDependency, 2> dependencies{
		//确保在开始子通道0之前，所有外部操作（如交换链图像获取、图像布局转换）都已完成。
		VkSubpassDependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_NONE_KHR,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		},
		//确保子通道0的渲染完成（且数据可用）后，外部操作（如图像呈现、后续渲染通道）才能开始。
		VkSubpassDependency{
			.srcSubpass = 0,
			.dstSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		}
	};

	VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subpassDescription,
		.dependencyCount = static_cast<uint32_t>(dependencies.size()),
		.pDependencies = dependencies.data()
	};

	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}
void ApplicationRaytracingBase::SetupFrameBuffer()
{
	m_frameBuffers.resize(m_swapChain.images.size());
	for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
		VkImageView attachments[2]{ m_swapChain.imageViews[i], m_depthStencil.view };
		VkFramebufferCreateInfo frameBufferCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_renderPass,// 这里的renderPass被改动过
			.attachmentCount = 2,
			.pAttachments = attachments,
			.width = width,
			.height = height,
			.layers = 1
		};
		VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]));
	}
}

void ApplicationRaytracingBase::EnableExtensions()
{
	apiVersion = VK_API_VERSION_1_3;
	m_enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	if (!rayQueryOnly) {
		m_enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	}

	m_enabledDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	m_enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	m_enabledDeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

	m_enabledDeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
	
	m_enabledFeatures.shaderStorageImageWriteWithoutFormat = VK_TRUE;
}

ApplicationRaytracingBase::ScratchBuffer ApplicationRaytracingBase::CreateScratchBuffer(VkDeviceSize size)
{
	ScratchBuffer scratchBuffer{};
	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.flags = 0,
		.size = size,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	VK_CHECK_RESULT(vkCreateBuffer(vulkanDevice->logicalDevice, &bufferCreateInfo, nullptr, &scratchBuffer.handle));
	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(vulkanDevice->logicalDevice, scratchBuffer.handle, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR
	};
	VkMemoryAllocateInfo memoryAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &memoryAllocateFlagsInfo,
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = vulkanDevice->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VK_CHECK_RESULT(vkAllocateMemory(vulkanDevice->logicalDevice, &memoryAllocateInfo, nullptr, &scratchBuffer.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(vulkanDevice->logicalDevice, scratchBuffer.handle, scratchBuffer.memory, 0));
	VkBufferDeviceAddressInfoKHR bufferDeviceAddresInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = scratchBuffer.handle
	};
	scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(vulkanDevice->logicalDevice, &bufferDeviceAddresInfo);
	return scratchBuffer;
}

void ApplicationRaytracingBase::DeleteScratchBuffer(ScratchBuffer& scratchBuffer)
{
	if (scratchBuffer.memory != VK_NULL_HANDLE) {
		vkFreeMemory(vulkanDevice->logicalDevice, scratchBuffer.memory, nullptr);
	}

	if (scratchBuffer.handle != VK_NULL_HANDLE) {
		vkDestroyBuffer(vulkanDevice->logicalDevice, scratchBuffer.handle, nullptr);
	}
}

void ApplicationRaytracingBase::CreateAccelerationStructure(AccelerationStructure& accelerationStructure,
	VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	VkBufferCreateInfo bufferCreateInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = buildSizeInfo.accelerationStructureSize,
		.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	VK_CHECK_RESULT(vkCreateBuffer(vulkanDevice->logicalDevice, &bufferCreateInfo, nullptr, &accelerationStructure.buffer));
	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(vulkanDevice->logicalDevice, accelerationStructure.buffer, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR
	};
	VkMemoryAllocateInfo memoryAllocateInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &memoryAllocateFlagsInfo,
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = vulkanDevice->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VK_CHECK_RESULT(vkAllocateMemory(vulkanDevice->logicalDevice, &memoryAllocateInfo, nullptr, &accelerationStructure.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(vulkanDevice->logicalDevice, accelerationStructure.buffer, accelerationStructure.memory, 0));
	VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.buffer = accelerationStructure.buffer,
		.size = buildSizeInfo.accelerationStructureSize,
		.type = type
	};
	vkCreateAccelerationStructureKHR(vulkanDevice->logicalDevice, &accelerationStructureCreate_info, nullptr, &accelerationStructure.handle);
	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.accelerationStructure = accelerationStructure.handle
	};
	accelerationStructure.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(vulkanDevice->logicalDevice, &accelerationDeviceAddressInfo);
}

void ApplicationRaytracingBase::DeleteAccelerationStructure(AccelerationStructure& accelerationStructure)
{
	vkFreeMemory(m_device, accelerationStructure.memory, nullptr);
	vkDestroyBuffer(m_device, accelerationStructure.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(m_device, accelerationStructure.handle, nullptr);
}

uint64_t ApplicationRaytracingBase::GetBufferDeviceAddress(VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAI{
	.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
	.buffer = buffer
	};
	return vkGetBufferDeviceAddressKHR(vulkanDevice->logicalDevice, &bufferDeviceAI);
}

void ApplicationRaytracingBase::CreateStorageImage(VkFormat format, VkExtent3D extent)
{
	// Release ressources if image is to be recreated
	if (m_storageImage.image != VK_NULL_HANDLE) {
		vkDestroyImageView(m_device, m_storageImage.view, nullptr);
		vkDestroyImage(m_device, m_storageImage.image, nullptr);
		vkFreeMemory(m_device, m_storageImage.memory, nullptr);
		m_storageImage = {};
	}

	VkImageCreateInfo image{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VK_CHECK_RESULT(vkCreateImage(vulkanDevice->logicalDevice, &image, nullptr, &m_storageImage.image));

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(vulkanDevice->logicalDevice, m_storageImage.image, &memReqs);
	VkMemoryAllocateInfo memoryAllocateInfo = Render::Vulkan::Initializer::MemoryAllocInfo();
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(vulkanDevice->logicalDevice, &memoryAllocateInfo, nullptr, &m_storageImage.memory));
	VK_CHECK_RESULT(vkBindImageMemory(vulkanDevice->logicalDevice, m_storageImage.image, m_storageImage.memory, 0));

	VkImageViewCreateInfo colorImageView{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = m_storageImage.image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
	};
	VK_CHECK_RESULT(vkCreateImageView(vulkanDevice->logicalDevice, &colorImageView, nullptr, &m_storageImage.view));

	VkCommandBuffer cmdBuffer = vulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	Render::Vulkan::Tool::SetImageLayout(cmdBuffer, m_storageImage.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	vulkanDevice->FlushCommandBuffer(cmdBuffer, m_queue);
}

void ApplicationRaytracingBase::DeleteStorageImage()
{
	vkDestroyImageView(vulkanDevice->logicalDevice, m_storageImage.view, nullptr);
	vkDestroyImage(vulkanDevice->logicalDevice, m_storageImage.image, nullptr);
	vkFreeMemory(vulkanDevice->logicalDevice, m_storageImage.memory, nullptr);
}

uint32_t alignedSize(uint32_t value, uint32_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

VkStridedDeviceAddressRegionKHR ApplicationRaytracingBase::GetSbtEntryStridedDeviceAddressRegion(VkBuffer buffer, uint32_t handleCount)
{
	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
	VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{
		.deviceAddress = GetBufferDeviceAddress(buffer),
		.stride = handleSizeAligned
	};
	stridedDeviceAddressRegionKHR.size = handleCount * handleSizeAligned;
	return stridedDeviceAddressRegionKHR;
}

void ApplicationRaytracingBase::CreateShaderBindingTable(ShaderBindingTable& shaderBindingTable, uint32_t handleCount)
{
	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&shaderBindingTable,
		rayTracingPipelineProperties.shaderGroupHandleSize * handleCount));

	shaderBindingTable.stridedDeviceAddressRegion = GetSbtEntryStridedDeviceAddressRegion(shaderBindingTable.buffer, handleCount);

	shaderBindingTable.Map();
}

void ApplicationRaytracingBase::DrawUI(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer)
{
	VkClearValue clearValues[2]{};
	clearValues[0].color = VkClearColorValue({ 0.025f, 0.025f, 0.025f, 1.0f });
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_renderPass,
		.framebuffer = framebuffer,
		.renderArea = {.offset = {.x = 0, .y = 0, }, .extent = {.width = width, .height = height } },
		.clearValueCount = 2,
		.pClearValues = clearValues
	};
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	ApplicationBase::DrawUI(commandBuffer);
	vkCmdEndRenderPass(commandBuffer);
}

void ApplicationRaytracingBase::Prepare()
{
	ApplicationBase::Prepare();

	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &rayTracingPipelineProperties
	};

	vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties2);
	accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	VkPhysicalDeviceFeatures2 deviceFeatures2{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &accelerationStructureFeatures
	};
	vkGetPhysicalDeviceFeatures2(m_physicalDevice, &deviceFeatures2);


	vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(m_device, "vkGetBufferDeviceAddressKHR"));
	vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_device, "vkCmdBuildAccelerationStructuresKHR"));
	vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(m_device, "vkBuildAccelerationStructuresKHR"));
	vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(m_device, "vkCreateAccelerationStructureKHR"));
	vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(m_device, "vkDestroyAccelerationStructureKHR"));
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(m_device, "vkGetAccelerationStructureBuildSizesKHR"));
	vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(m_device, "vkGetAccelerationStructureDeviceAddressKHR"));
	vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(m_device, "vkCmdTraceRaysKHR"));
	vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(m_device, "vkGetRayTracingShaderGroupHandlesKHR"));
	vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(m_device, "vkCreateRayTracingPipelinesKHR"));
}