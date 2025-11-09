#include "ApplicationWindow.h"

ApplicationWin::ApplicationWin():
	ApplicationBase()
{
	title = " ApplicationTraingle ";
}

ApplicationWin::~ApplicationWin()
{
	if (m_device) {
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
		vkDestroyBuffer(m_device, vertices.buffer, nullptr);
		vkFreeMemory(m_device, vertices.memory, nullptr);
		vkDestroyBuffer(m_device, indices.buffer, nullptr);
		vkFreeMemory(m_device, indices.memory, nullptr);
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		for (size_t i = 0; i < m_presentCompleteSemaphores.size(); i++) {
			vkDestroySemaphore(m_device, m_presentCompleteSemaphores[i], nullptr);
		}
		for (size_t i = 0; i < m_renderCompleteSemaphores.size(); i++) {
			vkDestroySemaphore(m_device, m_renderCompleteSemaphores[i], nullptr);
		}
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyFence(m_device, m_waitFences[i], nullptr);
			vkDestroyBuffer(m_device, m_uniformBuffers[i].buffer, nullptr);
			vkFreeMemory(m_device, m_uniformBuffers[i].memory, nullptr);
		}
	}
}

void ApplicationWin::CreateSynchronizationPrimitives()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkFenceCreateInfo fenceCI = Render::Vulkan::Initializer::FenceCreateInfo();
		fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCI, nullptr, &m_waitFences[i]));
	}
	m_presentCompleteSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	for (auto& semaphore : m_presentCompleteSemaphores) {
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
	}
	m_renderCompleteSemaphores.resize(m_swapChain.images.size());
	for (auto& semaphore : m_renderCompleteSemaphores) {
		VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
	}
}

void ApplicationWin::CreateCommandBuffers()
{
	VkCommandPoolCreateInfo commandPoolCI = Render::Vulkan::Initializer::CommandPoolCreateInfo();
	commandPoolCI.queueFamilyIndex = m_swapChain.queueNodeIndex;
	commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK_RESULT(vkCreateCommandPool(m_device, &commandPoolCI, nullptr, &m_commandPool));

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = Render::Vulkan::Initializer::CommandBufferAllocateInfo(m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT);
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, m_commandBuffers.data()));
}

void ApplicationWin::CreateVertexBuffer()
{
	std::vector<Vertex> vertexBuffer{
		{ {  1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
	};

	uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);
	std::vector<uint32_t> indexBuffer{ 0, 1, 2 };
	indices.count = static_cast<uint32_t>(indexBuffer.size());
	uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

	VkMemoryAllocateInfo memAlloc = Render::Vulkan::Initializer::MemoryAllocInfo();
	VkMemoryRequirements memReqs;

	struct StagingBuffer {
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	struct {
		StagingBuffer vertices;
		StagingBuffer indices;
	} stagingBuffers{};

	void* data;

	// Vertex buffer
	VkBufferCreateInfo vertexBufferInfoCI{};
	vertexBufferInfoCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfoCI.size = vertexBufferSize;
	// Buffer is used as the copy source
	vertexBufferInfoCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Create a host-visible buffer to copy the vertex data to (staging buffer)
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &vertexBufferInfoCI, nullptr, &stagingBuffers.vertices.buffer));
	vkGetBufferMemoryRequirements(m_device, stagingBuffers.vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	// Request a host visible memory type that can be used to copy our data to
	// Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &stagingBuffers.vertices.memory));
	// Map and copy
	VK_CHECK_RESULT(vkMapMemory(m_device, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data));
	memcpy(data, vertexBuffer.data(), vertexBufferSize);
	vkUnmapMemory(m_device, stagingBuffers.vertices.memory);
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0));

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	vertexBufferInfoCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &vertexBufferInfoCI, nullptr, &vertices.buffer));
	vkGetBufferMemoryRequirements(m_device, vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &vertices.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, vertices.buffer, vertices.memory, 0));

	// Index buffer
	VkBufferCreateInfo indexbufferCI{};
	indexbufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexbufferCI.size = indexBufferSize;
	indexbufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Copy index data to a buffer visible to the host (staging buffer)
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &indexbufferCI, nullptr, &stagingBuffers.indices.buffer));
	vkGetBufferMemoryRequirements(m_device, stagingBuffers.indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &stagingBuffers.indices.memory));
	VK_CHECK_RESULT(vkMapMemory(m_device, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data));
	memcpy(data, indexBuffer.data(), indexBufferSize);
	vkUnmapMemory(m_device, stagingBuffers.indices.memory);
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

	// Create destination buffer with device only visibility
	indexbufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &indexbufferCI, nullptr, &indices.buffer));
	vkGetBufferMemoryRequirements(m_device, indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &indices.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, indices.buffer, indices.memory, 0));

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
	VkCommandBuffer copyCmd;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = m_commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo, &copyCmd));

	VkCommandBufferBeginInfo cmdBufInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion{};
	// Vertex buffer
	copyRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, vertices.buffer, 1, &copyRegion);
	// Index buffer
	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, indices.buffer, 1, &copyRegion);
	VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

	// Submit the command buffer to the queue to finish the copy
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCI{};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.flags = 0;
	VkFence fence;
	VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCI, nullptr, &fence));

	// Submit to the queue
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, fence));
	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(m_device, fence, nullptr);
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(m_device, stagingBuffers.vertices.buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffers.vertices.memory, nullptr);
	vkDestroyBuffer(m_device, stagingBuffers.indices.buffer, nullptr);
	vkFreeMemory(m_device, stagingBuffers.indices.memory, nullptr);

}

void ApplicationWin::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> descriptorTypeCounts;
	descriptorTypeCounts.resize(1);
	descriptorTypeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorTypeCounts[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo descriptorPoolCI = Render::Vulkan::Initializer::DescriptorPoolCreateInfo(descriptorTypeCounts, MAX_FRAMES_IN_FLIGHT);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolCI, nullptr, &m_descriptorPool));
}

void ApplicationWin::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.pNext = nullptr;
	descriptorLayoutCI.bindingCount = 1;
	descriptorLayoutCI.pBindings = &layoutBinding;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayoutCI, nullptr, &m_descriptorSetLayout));
}

void ApplicationWin::CreateDescriptorSets()
{
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayout;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_uniformBuffers[i].descriptorSet));

		VkWriteDescriptorSet writeDescriptorSet{};

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i].buffer;
		bufferInfo.range = sizeof(ShaderData);

		// Binding 0 : Uniform buffer
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = m_uniformBuffers[i].descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &bufferInfo;
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(m_device, 1, &writeDescriptorSet, 0, nullptr);
	}
}

void ApplicationWin::SetupDepthStencil() 
{
	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = m_depthFormat;

	imageCI.extent = { width, height, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VK_CHECK_RESULT(vkCreateImage(m_device, &imageCI, nullptr, &m_depthStencil.image));

	// Allocate memory for the image (device local) and bind it to our image
	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(m_device, m_depthStencil.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAlloc, nullptr, &m_depthStencil.memory));
	VK_CHECK_RESULT(vkBindImageMemory(m_device, m_depthStencil.image, m_depthStencil.memory, 0));

	// Create a view for the depth stencil image
	// Images aren't directly accessed in Vulkan, but rather through views described by a subresource range
	// This allows for multiple views of one image with differing ranges (e.g. for different layers)
	VkImageViewCreateInfo depthStencilViewCI{};
	depthStencilViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilViewCI.format = m_depthFormat;
	depthStencilViewCI.subresourceRange = {};
	depthStencilViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT)
	if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		depthStencilViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	depthStencilViewCI.subresourceRange.baseMipLevel = 0;
	depthStencilViewCI.subresourceRange.levelCount = 1;
	depthStencilViewCI.subresourceRange.baseArrayLayer = 0;
	depthStencilViewCI.subresourceRange.layerCount = 1;
	depthStencilViewCI.image = m_depthStencil.image;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &depthStencilViewCI, nullptr, &m_depthStencil.view));
}

// Create a frame buffer for each swap chain image
// Note: Override of virtual function in the base class and called from within VulkanExampleBase::prepare
void ApplicationWin::SetupFrameBuffer() 
{
	// Create a frame buffer for every image in the swapchain
	m_frameBuffers.resize(m_swapChain.images.size());
	for (size_t i = 0; i < m_frameBuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments{};
		// Color attachment is the view of the swapchain image
		attachments[0] = m_swapChain.imageViews[i];
		// Depth/Stencil attachment is the same for all frame buffers due to how depth works with current GPUs
		attachments[1] = m_depthStencil.view;

		VkFramebufferCreateInfo frameBufferCI{};
		frameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		// All frame buffers use the same renderpass setup
		frameBufferCI.renderPass = m_renderPass;
		frameBufferCI.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCI.pAttachments = attachments.data();
		frameBufferCI.width = width;
		frameBufferCI.height = height;
		frameBufferCI.layers = 1;
		// Create the framebuffer
		VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &frameBufferCI, nullptr, &m_frameBuffers[i]));
	}
}

void ApplicationWin::SetupRenderPass() 
{
	// This example will use a single render pass with one subpass

	// Descriptors for the attachments used by this renderpass
	std::array<VkAttachmentDescription, 2> attachments{};

	// Color attachment
	attachments[0].format = m_swapChain.colorFormat;                                  // Use the color format selected by the swapchain
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;                                 // We don't use multi sampling in this example
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                            // Clear this attachment at the start of the render pass
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;                          // Keep its contents after the render pass is finished (for displaying it)
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                 // We don't use stencil, so don't care for load
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;               // Same for store
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                       // Layout at render pass start. Initial doesn't matter, so we use undefined
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                   // Layout to which the attachment is transitioned when the render pass is finished

	attachments[1].format = m_depthFormat;                                           // A proper depth format is selected in the example base
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // Clear depth at start of first subpass
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // We don't need depth after render pass has finished (DONT_CARE may result in better performance)
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // No stencil
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // No Stencil
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Layout at render pass start. Initial doesn't matter, so we use undefined
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Transition to depth/stencil attachment

	// Setup attachment references
	VkAttachmentReference colorReference{};
	colorReference.attachment = 0;                                    // Attachment 0 is color
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Attachment layout used as color during the subpass

	VkAttachmentReference depthReference{};
	depthReference.attachment = 1;                                            // Attachment 1 is color
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Attachment used as depth/stencil used during the subpass

	// Setup a single subpass reference
	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;                            // Subpass uses one color attachment
	subpassDescription.pColorAttachments = &colorReference;                 // Reference to the color attachment in slot 0
	subpassDescription.pDepthStencilAttachment = &depthReference;           // Reference to the depth attachment in slot 1
	subpassDescription.inputAttachmentCount = 0;                            // Input attachments can be used to sample from contents of a previous subpass
	subpassDescription.pInputAttachments = nullptr;                         // (Input attachments not used by this example)
	subpassDescription.preserveAttachmentCount = 0;                         // Preserved attachments can be used to loop (and preserve) attachments through subpasses
	subpassDescription.pPreserveAttachments = nullptr;                      // (Preserve attachments not used by this example)
	subpassDescription.pResolveAttachments = nullptr;                       // Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling

	// Setup subpass dependencies
	// These will add the implicit attachment layout transitions specified by the attachment descriptions
	// The actual usage layout is preserved through the layout specified in the attachment reference
	// Each subpass dependency will introduce a memory and execution dependency between the source and dest subpass described by
	// srcStageMask, dstStageMask, srcAccessMask, dstAccessMask (and dependencyFlags is set)
	// Note: VK_SUBPASS_EXTERNAL is a special constant that refers to all commands executed outside of the actual renderpass)
	std::array<VkSubpassDependency, 2> dependencies{};

	// Does the transition from final to initial layout for the depth an color attachments
	// Depth attachment
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[0].dependencyFlags = 0;
	// Color attachment
	dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].dstSubpass = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = 0;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].dependencyFlags = 0;

	// Create the actual renderpass
	VkRenderPassCreateInfo renderPassCI{};
	renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCI.attachmentCount = static_cast<uint32_t>(attachments.size());  // Number of attachments used by this render pass
	renderPassCI.pAttachments = attachments.data();                            // Descriptions of the attachments used by the render pass
	renderPassCI.subpassCount = 1;                                             // We only use one subpass in this example
	renderPassCI.pSubpasses = &subpassDescription;                             // Description of that subpass
	renderPassCI.dependencyCount = static_cast<uint32_t>(dependencies.size()); // Number of subpass dependencies
	renderPassCI.pDependencies = dependencies.data();                          // Subpass dependencies used by the render pass
	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassCI, nullptr, &m_renderPass));
}

VkShaderModule ApplicationWin::LoadSPIRVShader(const std::string& filename)
{
	size_t shaderSize;
	char* shaderCode{ nullptr };

#if defined (__ANDROID__)
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager,filename.c_str(), AASSET_MODE_STREAMING );
    assert(asset);
    shaderSize = AAsset_getLength(asset);
    assert(shaderSize > 0);

    shaderCode = new char[shaderSize];
    AAsset_read(asset, shaderCode, shaderSize);
    AAsset_close(asset);
#else
	std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (is.is_open())
	{
		shaderSize = is.tellg();
		is.seekg(0, std::ios::beg);
		// Copy file contents into a buffer
		shaderCode = new char[shaderSize];
		is.read(shaderCode, shaderSize);
		is.close();
		assert(shaderSize > 0);
	}
#endif
	if (shaderCode)
	{
		// Create a new shader module that will be used for pipeline creation
		VkShaderModuleCreateInfo shaderModuleCI{};
		shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCI.codeSize = shaderSize;
		shaderModuleCI.pCode = (uint32_t*)shaderCode;

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(m_device, &shaderModuleCI, nullptr, &shaderModule));

		delete[] shaderCode;

		return shaderModule;
	}
	else
	{
		std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
		return VK_NULL_HANDLE;
	}
}

void ApplicationWin::CreatePipelines()
{
	// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
	// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.pNext = nullptr;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout));

	// Create the graphics pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
	// A pipeline is then stored and hashed on the GPU making pipeline changes very fast
	// Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
	pipelineCI.layout = m_pipelineLayout;
	// Renderpass this pipeline is attached to
	pipelineCI.renderPass = m_renderPass;

	// Construct the different states making up the pipeline

	// Input assembly state describes how primitives are assembled
	// This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
	inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
	rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
	rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCI.depthClampEnable = VK_FALSE;
	rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCI.depthBiasEnable = VK_FALSE;
	rasterizationStateCI.lineWidth = 1.0f;

	// Color blend state describes how blend factors are calculated (if used)
	// We need one blend attachment state per color attachment (even if blending is not used)
	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = 0xf;
	blendAttachmentState.blendEnable = VK_FALSE;
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
	colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCI.attachmentCount = 1;
	colorBlendStateCI.pAttachments = &blendAttachmentState;

	// Viewport state sets the number of viewports and scissor used in this pipeline
	// Note: This is actually overridden by the dynamic states (see below)
	VkPipelineViewportStateCreateInfo viewportStateCI{};
	viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCI.viewportCount = 1;
	viewportStateCI.scissorCount = 1;

	// Enable dynamic states
	// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
	// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
	// For this example we will set the viewport and scissor using dynamic states
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
	dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Depth and stencil state containing depth and stencil compare and test operations
	// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
	depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCI.depthTestEnable = VK_TRUE;
	depthStencilStateCI.depthWriteEnable = VK_TRUE;
	depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCI.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCI.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStateCI.stencilTestEnable = VK_FALSE;
	depthStencilStateCI.front = depthStencilStateCI.back;

	// Multi sampling state
	// This example does not make use of multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
	VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCI.pSampleMask = nullptr;

	// Vertex input descriptions
	// Specifies the vertex input parameters for a pipeline

	// Vertex input binding
	// This example uses a single vertex input binding at binding point 0 (see vkCmdBindVertexBuffers)
	VkVertexInputBindingDescription vertexInputBinding{};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Input attribute bindings describe shader attribute locations and memory layouts
	std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs{};

	vertexInputAttributs[0].binding = 0;
	vertexInputAttributs[0].location = 0;
	// Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[0].offset = offsetof(Vertex, position);
	// Attribute location 1: Color
	vertexInputAttributs[1].binding = 0;
	vertexInputAttributs[1].location = 1;
	// Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[1].offset = offsetof(Vertex, color);

	// Vertex input state used for pipeline creation
	VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
	vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCI.vertexBindingDescriptionCount = 1;
	vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputStateCI.vertexAttributeDescriptionCount = 2;
	vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributs.data();

	// Shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

#if defined( __ANDROID__)
    std::string  vertexShaderPath = "shaders/glsl/triangle/triangle.vert.spv";
    std::string  fragShaderPath = "shaders/glsl/triangle/triangle.frag.spv";
#else
    std::string  vertexShaderPath = "./Asset/shader/glsl/triangle/triangle.vert.spv";
    std::string  fragShaderPath = "./Asset/shader/glsl/triangle/triangle.frag.spv";
#endif

	// Vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// Set pipeline stage for this shader
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	// Load binary SPIR-V shader
	shaderStages[0].module = LoadSPIRVShader(vertexShaderPath.c_str());
	// Main entry point for the shader
	shaderStages[0].pName = "main";
	assert(shaderStages[0].module != VK_NULL_HANDLE);

	// Fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// Set pipeline stage for this shader
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	// Load binary SPIR-V shader
	shaderStages[1].module = LoadSPIRVShader(fragShaderPath.c_str());
	// Main entry point for the shader
	shaderStages[1].pName = "main";
	assert(shaderStages[1].module != VK_NULL_HANDLE);

	// Set pipeline shader stage info
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	// Assign the pipeline states to the pipeline creation info structure
	pipelineCI.pVertexInputState = &vertexInputStateCI;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &colorBlendStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;

	// Create rendering pipeline using the specified states
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineCI, nullptr, &m_pipeline));

	// Shader modules are no longer needed once the graphics pipeline has been created
	vkDestroyShaderModule(m_device, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device, shaderStages[1].module, nullptr);
}

void ApplicationWin::CreateUniformBuffers()
{
	// Prepare and initialize the per-frame uniform buffer blocks containing shader uniforms
	// Single uniforms like in OpenGL are no longer present in Vulkan. All hader uniforms are passed via uniform buffer blocks
	VkMemoryRequirements memReqs;

	// Vertex shader uniform buffer block
	VkBufferCreateInfo bufferInfo{};
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(ShaderData);
	// This buffer will be used as a uniform buffer
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	// Create the buffers
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_uniformBuffers[i].buffer));
		// Get memory requirements including size, alignment and memory type
		vkGetBufferMemoryRequirements(m_device, m_uniformBuffers[i].buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		// Get the memory type index that supports host visible memory access
		// Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
		// We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
		// Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
		allocInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		// Allocate memory for the uniform buffer
		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &(m_uniformBuffers[i].memory)));
		// Bind memory to buffer
		VK_CHECK_RESULT(vkBindBufferMemory(m_device, m_uniformBuffers[i].buffer, m_uniformBuffers[i].memory, 0));
		// We map the buffer once, so we can update it without having to map it again
		VK_CHECK_RESULT(vkMapMemory(m_device, m_uniformBuffers[i].memory, 0, sizeof(ShaderData), 0, (void**)&m_uniformBuffers[i].mapped));
	}

}

void ApplicationWin::Prepare() 
{
	ApplicationBase::Prepare();
	CreateSynchronizationPrimitives();
	CreateCommandBuffers();
	CreateVertexBuffer();
	CreateUniformBuffers();
	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreatePipelines();
	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{
	//m_camera.setPerspective(60.0f, (float)(width / 3.0f) / (float)height, 0.1f, 256.0f);

	ShaderData shaderData{};
	shaderData.projectionMatrix = m_camera.matrices.perspective;
	shaderData.viewMatrix = m_camera.matrices.view;
	shaderData.modelMatrix = glm::mat4(1.0f);

	memcpy(m_uniformBuffers[m_currentFrame].mapped, &shaderData, sizeof(ShaderData));
}

void ApplicationWin::Render()
{
	if (!prepared)
		return;

	// Use a fence to wait until the command buffer has finished execution before using it again
	vkWaitForFences(m_device, 1, &m_waitFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_waitFences[m_currentFrame]));

	// Get the next swap chain image from the implementation
	// Note that the implementation is free to return the images in any order, so we must use the acquire function and can't just cycle through the images/imageIndex on our own
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain.swapChain, UINT64_MAX, m_presentCompleteSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		WindowResize();
		return;
	}
	else if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR)) {
		throw "Could not acquire the next swap chain image!";
	}

	UpdateUniformBuffers();

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

	VkCommandBufferBeginInfo cmdBufInfo{};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2]{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = m_frameBuffers[imageIndex];

	const VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

	// Start the first sub pass specified in our default render pass setup by the base class
	// This will clear the color and depth attachment
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	// Update dynamic viewport state
	VkViewport viewport{};
	viewport.height = (float)height;
	viewport.width = (float)width;
	viewport.minDepth = (float)0.0f;
	viewport.maxDepth = (float)1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	// Update dynamic scissor state
	VkRect2D scissor{};
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	// Bind descriptor set for the current frame's uniform buffer, so the shader uses the data from that buffer for this draw
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_uniformBuffers[m_currentFrame].descriptorSet, 0, nullptr);
	// Bind the rendering pipeline
	// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	// Bind triangle vertex buffer (contains position and colors)
	VkDeviceSize offsets[1]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
	// Bind triangle index buffer
	vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
	// Draw indexed triangle
	vkCmdDrawIndexed(commandBuffer, indices.count, 1, 0, 0, 0);

	DrawUI(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
	// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system
	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	// Submit the command buffer to the graphics queue

	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// The submit info structure specifies a command buffer queue submission batch
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStageMask;      // Pointer to the list of pipeline stages that the semaphore waits will occur at
	submitInfo.pCommandBuffers = &commandBuffer;		// Command buffers(s) to execute in this batch (submission)
	submitInfo.commandBufferCount = 1;                  // We submit a single command buffer

	// Semaphore to wait upon before the submitted command buffer starts executing
	submitInfo.pWaitSemaphores = &m_presentCompleteSemaphores[m_currentFrame];
	submitInfo.waitSemaphoreCount = 1;
	// Semaphore to be signaled when command buffers have completed
	submitInfo.pSignalSemaphores = &m_renderCompleteSemaphores[imageIndex];
	submitInfo.signalSemaphoreCount = 1;

	// Submit to the graphics queue passing a wait fence
	VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, m_waitFences[m_currentFrame]));

	// Present the current frame buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderCompleteSemaphores[imageIndex];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapChain.swapChain;
	presentInfo.pImageIndices = &imageIndex;
	result = vkQueuePresentKHR(m_queue, &presentInfo);

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		WindowResize();
	}
	else if (result != VK_SUCCESS) {
		throw "Could not present the image to the swap chain!";
	}

	// Select the next frame to render to, based on the max. no. of concurrent frames
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	vkQueueWaitIdle(m_queue);//wait queue wheter UI destroy is error !!!!
}