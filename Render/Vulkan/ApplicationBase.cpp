#include "ApplicationBase.h"

#if (defined(VK_USE_PLATFORM_METAL_EXT))
#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#include <CoreVideo/CVDisplayLink.h>
#endif

std::string ApplicationBase::GetWindowTitle() const
{
	std::string windowTitle{ title + " - " + m_deviceProperties.deviceName };
	return windowTitle;
}

bool ApplicationBase::InitVulkan()
{
	VkResult result = CreateInstance();
	if (result != VK_SUCCESS)
	{
		std::runtime_error("Create Instance !!!");
        return false;
	}

	if (debugVulkan)
	{
		Render::Vulkan::Debug::SetupDebugging(m_instance);
	}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    Render::Vulkan::android::LoadVulkanFunctions(m_instance);
#endif

	// Physical device
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
	if (gpuCount == 0) {
		std::runtime_error ("No device with Vulkan support found");
		return false;
	}
	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	result = vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());
	if (result != VK_SUCCESS) {
		std::runtime_error("Could not enumerate physical devices : \n");
		return false;
	}

	// GPU selection

	// Select physical device to be used for the Vulkan example

	uint32_t selectedDevice = 0;
	std::vector<VkPhysicalDevice> devices(gpuCount);
	vkEnumeratePhysicalDevices(m_instance, &gpuCount, devices.data());
	for (int i = 0; i < gpuCount; i++)
	{
		selectedDevice = i;
		break;
	}

	m_physicalDevice = physicalDevices[selectedDevice];

	// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

	// Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
	GetEnabledFeatures();

	vulkanDevice = new Render::Vulkan::VulkanDevice(m_physicalDevice);

	GetEnabledExtensions();

	result = vulkanDevice->CreateLogicalDevice(m_enabledFeatures, m_enabledDeviceExtensions, m_deviceCreatepNextChain);
	if (result != VK_SUCCESS) {
		std::runtime_error("Could not create Vulkan device: \n");
		return false;
	}
	m_device = vulkanDevice->logicalDevice;

	// Get a graphics queue from the device
	vkGetDeviceQueue(m_device, vulkanDevice->queueFamilyIndices.graphics, 0, &m_queue);

	// Find a suitable depth and/or stencil format
	VkBool32 validFormat{ false };
	// Samples that make use of stencil will require a depth + stencil format, so we select from a different list
	if (requiresStencil) {
		validFormat = Render::Vulkan::Tool::GetSupportedDepthStencilFormat(m_physicalDevice, &m_depthFormat);
	}
	else {
		validFormat = Render::Vulkan::Tool::GetSupportedDepthFormat(m_physicalDevice, &m_depthFormat);
	}
	assert(validFormat);

	m_swapChain.SetContext(m_instance, m_physicalDevice, m_device);

	return true;
}

void ApplicationBase::GetEnabledFeatures() {}

void ApplicationBase::GetEnabledExtensions() {}

VkResult ApplicationBase::CreateInstance()
{
	std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

#if defined (_WIN32)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif

	uint32_t extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		{
			for (VkExtensionProperties& extension : extensions)
			{
				m_supportedInstanceExtensions.push_back(extension.extensionName);
			}
		}
	}
    
#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT))
    // SRS - When running on iOS/macOS with MoltenVK, enable VK_KHR_get_physical_device_properties2 if not already enabled by the example (required by VK_KHR_portability_subset)
    if (std::find(m_enabledInstanceExtensions.begin(), m_enabledInstanceExtensions.end(), VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == m_enabledInstanceExtensions.end())
    {
        m_enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
#endif

	if (!m_enabledInstanceExtensions.empty())
	{
		for (const char* enabledExtension : m_enabledInstanceExtensions)
		{
			// Output message if requested extension is not available
			if (std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), enabledExtension) == m_supportedInstanceExtensions.end())
			{
				std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
			}
			instanceExtensions.push_back(enabledExtension);
		}
	}

	if (useGlslang)
	{
		if (apiVersion < VK_API_VERSION_1_1) {
			apiVersion = VK_API_VERSION_1_1;
		}
		m_enabledDeviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
		m_enabledDeviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
		m_enabledDeviceExtensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
	}

	VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = name.c_str(),
		.pEngineName = name.c_str(),
		.apiVersion = apiVersion
	};

	VkInstanceCreateInfo instanceCreateInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo
	};

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
	if (debugVulkan) {
		Render::Vulkan::Debug::SetupDebugingMessengerCreateInfo(debugUtilsMessengerCI);
		debugUtilsMessengerCI.pNext = instanceCreateInfo.pNext;
		instanceCreateInfo.pNext = &debugUtilsMessengerCI;
	}


	// Enable the debug utils extension if available (e.g. when debugging tools are present)
	if (debugVulkan || std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != m_supportedInstanceExtensions.end()) {
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
    
#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)) && defined(VK_KHR_portability_enumeration)
    if (std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != m_supportedInstanceExtensions.end())
    {
        instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

	if (!instanceExtensions.empty()) {
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
	if (debugVulkan) {
		// Check if this layer is available at instance level
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties& layer : instanceLayerProperties) {
			if (strcmp(layer.layerName, validationLayerName) == 0) {
				validationLayerPresent = true;
				break;
			}
		}
		if (validationLayerPresent) {
			instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
			instanceCreateInfo.enabledLayerCount = 1;
		}
		else {
			std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
		}
	}

	VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo{ .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT };
	if (m_enabledLayerSettings.size() > 0) {
		layerSettingsCreateInfo.settingCount = static_cast<uint32_t>(m_enabledLayerSettings.size());
		layerSettingsCreateInfo.pSettings = m_enabledLayerSettings.data();
		layerSettingsCreateInfo.pNext = instanceCreateInfo.pNext;
		instanceCreateInfo.pNext = &layerSettingsCreateInfo;
	}

	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);

	if (std::find(m_supportedInstanceExtensions.begin(), m_supportedInstanceExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != m_supportedInstanceExtensions.end()) {
		Render::Vulkan::DebugUtils::Setup(m_instance);
	}

	return result;
}

void ApplicationBase::CreateSurface()
{
#if defined (_WIN32)
	m_swapChain.InitSurface(windowInstance, window);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    m_swapChain.InitSurface(androidApp->window);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    m_swapChain.InitSurface(metalLayer);
#endif
}

void ApplicationBase::CreateCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo = Render::Vulkan::Initializer::CommandPoolCreateInfo();
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = m_swapChain.queueNodeIndex;
	VK_CHECK_RESULT(vkCreateCommandPool(m_device, &cmdPoolCreateInfo, nullptr, &m_cmdPool));
}

void ApplicationBase::CreateSwapChain()
{
	m_swapChain.Create(width, height, useVsync, useFullScreen);
}

void ApplicationBase::CreateCommandBuffers()
{
	VkCommandBufferAllocateInfo cmdAllocInfo = Render::Vulkan::Initializer::CommandBufferAllocateInfo(m_cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_drawCmdBuffers.size());
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, m_drawCmdBuffers.data()));
}

void ApplicationBase::CreateSynchronizationPrimitives()
{
	VkFenceCreateInfo fenceCreateInfo = Render::Vulkan::Initializer::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	for (auto& fence : m_waitFences)
		VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence));

	for (auto& semaphore : m_presentCompleteSemaphores)
	{
		VkSemaphoreCreateInfo semaphoreCI = Render::Vulkan::Initializer::SemaphoreCreateInfo();
		VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
	}

	m_renderCompleteSemaphores.resize(m_swapChain.images.size());
	for (auto& semaphore : m_renderCompleteSemaphores)
	{
		VkSemaphoreCreateInfo semaphoreCI = Render::Vulkan::Initializer::SemaphoreCreateInfo();
		VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &semaphore));
	}
}

void ApplicationBase::DestroyCommandBuffers()
{
	vkFreeCommandBuffers(m_device, m_cmdPool, static_cast<uint32_t>(m_drawCmdBuffers.size()), m_drawCmdBuffers.data());
}

void ApplicationBase::SetupDepthStencil()
{
	VkImageCreateInfo imageCI = Render::Vulkan::Initializer::ImageCreateInfo();
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
	VkMemoryRequirements memReq{};
	vkGetImageMemoryRequirements(m_device, m_depthStencil.image, &memReq);

	VkMemoryAllocateInfo memAllloc = Render::Vulkan::Initializer::MemoryAllocInfo();
	memAllloc.allocationSize = memReq.size;
	memAllloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memAllloc, nullptr, &m_depthStencil.memory));
	VK_CHECK_RESULT(vkBindImageMemory(m_device, m_depthStencil.image, m_depthStencil.memory, 0));

	VkImageViewCreateInfo imageViewCI = Render::Vulkan::Initializer::ImageViewCreateInfo();
	imageViewCI.image = m_depthStencil.image;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.format = m_depthFormat;
	imageViewCI.subresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VK_CHECK_RESULT(vkCreateImageView(m_device, &imageViewCI, nullptr, &m_depthStencil.view));
}

void ApplicationBase::SetupRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachments{
		//Color Attachemnt
		VkAttachmentDescription{
			.format = m_swapChain.colorFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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

	VkAttachmentReference colorReference{ .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference{ .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpassDescription
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorReference,
		.pDepthStencilAttachment = &depthReference
	};

	std::array<VkSubpassDependency, 2> dependencies{
					VkSubpassDependency{
						.srcSubpass = VK_SUBPASS_EXTERNAL,
						.dstSubpass = 0,
						.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
						.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
					},
					VkSubpassDependency{
						.srcSubpass = VK_SUBPASS_EXTERNAL,
						.dstSubpass = 0,
						.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						.srcAccessMask = 0,
						.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
					},
	};

	VkRenderPassCreateInfo renderPassInfo = Render::Vulkan::Initializer::RenderPassCreateInfo();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}

void ApplicationBase::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	VK_CHECK_RESULT(vkCreatePipelineCache(m_device, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
}

void ApplicationBase::SetupFrameBuffer()
{
	m_frameBuffers.resize(m_swapChain.images.size());
	for (uint32_t i = 0;i < m_frameBuffers.size();i++)
	{
		const VkImageView attachments[2] = { m_swapChain.imageViews[i], m_depthStencil.view };
		VkFramebufferCreateInfo frameBufferCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_renderPass,
			.attachmentCount = 2,
			.pAttachments = attachments,
			.width = width,
			.height = height,
			.layers = 1
		};
		VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]));
	}
}


void ApplicationBase::BuildCommandBuffer()
{
}

void ApplicationBase::OnUpdateUIOverlay(UIOverlay* ui)
{
}

void ApplicationBase::NextFrame()
{
	Render();
	frameCounter++;
	UpdateOverlay();
}

void ApplicationBase::UpdateOverlay()
{

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)width, (float)height);

	io.MousePos = ImVec2(mouseState.position.x, mouseState.position.y);
	io.MouseDown[0] = mouseState.button.left;
	io.MouseDown[1] = mouseState.button.right;
	io.MouseDown[2] = mouseState.button.middle;

	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(10 * ui.scale, 10 * ui.scale));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::TextUnformatted(title.c_str());
	ImGui::TextUnformatted(m_deviceProperties.deviceName);
	ImGui::Text("Triangle Demo");
	ImGui::PushItemWidth(250.0f * ui.scale);
	OnUpdateUIOverlay(&ui);
	ImGui::PopItemWidth();

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::Render();

	ui.Update();
}

void ApplicationBase::SubmitFrame(bool skipQueueSubmit)
{
	if (!skipQueueSubmit) {
		const VkPipelineStageFlags waitPipelineStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_presentCompleteSemaphores[m_currentBuffer],
			.pWaitDstStageMask = &waitPipelineStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_drawCmdBuffers[m_currentBuffer],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &m_renderCompleteSemaphores[m_currentImageIndex]
		};
		VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submitInfo, m_waitFences[m_currentBuffer]));
	}

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderCompleteSemaphores[m_currentImageIndex],
		.swapchainCount = 1,
		.pSwapchains = &m_swapChain.swapChain,
		.pImageIndices = &m_currentImageIndex
	};
	VkResult result = vkQueuePresentKHR(m_queue, &presentInfo);

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
		WindowResize();
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return;
		}
	}
	else {
		VK_CHECK_RESULT(result);
	}

	m_currentBuffer = (m_currentBuffer + 1) % MAX_FRAMES_IN_FLIGHT;
}

void ApplicationBase::Prepare()
{
	CreateSurface();
	CreateCommandPool();
	CreateSwapChain();
	CreateCommandBuffers();
	CreateSynchronizationPrimitives();
	SetupDepthStencil();
	SetupRenderPass();
	CreatePipelineCache();
	SetupFrameBuffer();



	VkPipelineShaderStageCreateInfo shaderVertStage = {};
	shaderVertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderVertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
#if defined (__ANDROID__)
    shaderVertStage.module = Render::Vulkan::Tool::LoadShader(androidApp->activity->assetManager, "shaders/glsl/base/uioverlay.vert.spv",m_device);
#else
    shaderVertStage.module = Render::Vulkan::Tool::LoadShader("./Asset/shader/glsl/base/uioverlay.vert.spv",m_device);
#endif

	shaderVertStage.pName = "main";

	VkPipelineShaderStageCreateInfo shaderFragStage = {};
	shaderFragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderFragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    
#if defined (__ANDROID__)
    shaderFragStage.module = Render::Vulkan::Tool::LoadShader(androidApp->activity->assetManager, "shaders/glsl/base/uioverlay.frag.spv", m_device);
#else
    shaderFragStage.module = Render::Vulkan::Tool::LoadShader("./Asset/shader/glsl/base/uioverlay.frag.spv", m_device);
#endif
	shaderFragStage.pName = "main";

	ui.device = vulkanDevice;
	ui.queue = m_queue;
	ui.shaders = {
		shaderVertStage,
		shaderFragStage
	};

	ui.PrepareResource();
	ui.PreparePipeline(m_pipelineCache, m_renderPass, m_swapChain.colorFormat, m_depthFormat);
}

void ApplicationBase::DrawUI(const VkCommandBuffer commandBuffer)
{
	const VkViewport viewport = Render::Vulkan::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
	const VkRect2D scissor = Render::Vulkan::Initializer::Rect2D(width, height, 0, 0);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	ui.Draw(commandBuffer);
}

void ApplicationBase::RenderLoop()
{

	destWidth = width;
	destHeight = height;

#if defined(_WIN32)
	MSG msg;
	bool quitMessageReceived = false;
	while (!quitMessageReceived) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				quitMessageReceived = true;
				break;
			}
		}
		if (prepared && !IsIconic(window)) {
			NextFrame();
		}
	}
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    [NSApp run];
#elif defined(__ANDROID__)

    while(true)
    {
        int ident;
        int events;
        struct android_poll_source* source;
        bool destroy = false;

        focused = true;

        while ((ident = ALooper_pollOnce(focused ? 0 : -1, nullptr, &events,
                                         (void **) &source)) > ALOOPER_POLL_TIMEOUT) {
            if (source != nullptr) {
                source->process(androidApp, source);
            }
            if (androidApp->destroyRequested != 0) {
                LOGD("Android app destroy requested");
                destroy = true;
                break;
            }
        }



        if (destroy)
        {
            ANativeActivity_finish(androidApp->activity);
            break;
        }


        if(prepared)
        {
            Render();
            UpdateOverlay();
        }
    }
#endif
}

void ApplicationBase::WindowResize()
{
	prepared = false;

	vkDeviceWaitIdle(m_device);

	width = destWidth;
	height = destHeight;
	CreateSwapChain();

	//Recreate the frame buffer
	vkDestroyImageView(m_device, m_depthStencil.view, nullptr);
	vkDestroyImage(m_device, m_depthStencil.image, nullptr);
	vkFreeMemory(m_device, m_depthStencil.memory, nullptr);
	SetupDepthStencil();
	
	for (auto& frameBuffer : m_frameBuffers)
		vkDestroyFramebuffer(m_device, frameBuffer, nullptr);

	SetupFrameBuffer();

	for (auto& fence : m_waitFences)
		vkDestroyFence(m_device, fence, nullptr);

	for (auto semp : m_presentCompleteSemaphores)
		vkDestroySemaphore(m_device, semp, nullptr);

	for (auto semp : m_renderCompleteSemaphores)
		vkDestroySemaphore(m_device, semp, nullptr);

	ui.Resize(width, height);
	BuildCommandBuffer();
	CreateSynchronizationPrimitives();

	m_camera.updateAspectRatio((float)width / (float)height);
	vkDeviceWaitIdle(m_device);
	prepared = true;
}

void ApplicationBase::LoadGlTFFile(std::string path)
{
	/*
	tinygltf::Model glTFInput;
	tinygltf::TinyGLTF gltfContext;
	std::string error, warning;

	bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, path);

	m_glTFModel.vulkanDevice = this->vulkanDevice;
	m_glTFModel.copyQueue = m_queue;

	std::vector<uint32_t> indexBuffer;
	std::vector<GlTFModel::Vertex> vertexBuffer;

	if (fileLoaded)
	{
		m_glTFModel.LoadImages(glTFInput);
		m_glTFModel.LoadMaterials(glTFInput);
		m_glTFModel.LoadTextures(glTFInput);

		const tinygltf::Scene& scene = glTFInput.scenes[0];
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
			m_glTFModel.LoadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
		}
	}
	else
	{
		return;
	}

	size_t vertexBufferSize = vertexBuffer.size() * sizeof(GlTFModel::Vertex);
	size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
	m_glTFModel.indices.count = static_cast<uint32_t>(indexBuffer.size());

	struct StagingBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	} vertexStaging, indexStaging;

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		vertexBufferSize,
		&vertexStaging.buffer,
		&vertexStaging.memory,
		vertexBuffer.data()
	));

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		indexBufferSize,
		&indexStaging.buffer,
		&indexStaging.memory,
		indexBuffer.data()));

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBufferSize,
		&m_glTFModel.vertices.buffer,
		&m_glTFModel.vertices.memory));
	
	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBufferSize,
		&m_glTFModel.indices.buffer,
		&m_glTFModel.indices.memory));

	VkCommandBuffer copyCmd = vulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	VkBufferCopy copyRegion = {};

	copyRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(
		copyCmd,
		vertexStaging.buffer,
		m_glTFModel.vertices.buffer,
		1,
		&copyRegion);

	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(
		copyCmd,
		indexStaging.buffer,
		m_glTFModel.indices.buffer,
		1,
		&copyRegion);

	vulkanDevice->FlushCommandBuffer(copyCmd, m_queue, true);
	vkDestroyBuffer(m_device, vertexStaging.buffer, nullptr);
	vkFreeMemory(m_device, vertexStaging.memory, nullptr);
	vkDestroyBuffer(m_device, indexStaging.buffer, nullptr);
	vkFreeMemory(m_device, indexStaging.memory, nullptr);
	*/
}

VkPipelineShaderStageCreateInfo ApplicationBase::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage{
	.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	.stage = stage,
	.pName = "main"
	};

#if defined (__ANDROID__)
	shaderStage.module = Render::Vulkan::Tool::LoadShader(androidApp->activity->assetManager, fileName.c_str(), m_device);
#else
	shaderStage.module = Render::Vulkan::Tool::LoadShader(fileName.c_str(), m_device);
#endif
	assert(shaderStage.module != VK_NULL_HANDLE);
	m_shaderModules.push_back(shaderStage.module);
	return shaderStage;
}

ApplicationBase::ApplicationBase()
{
#if defined(_WIN32)
	if (debugVulkan)
	{
		SetupConsole("Debug Console");
	}
#elif defined(__ANDROID__)
    bool libLoaded = Render::Vulkan::android::LoadVulkanLibrary();
    assert(libLoaded);
#endif

	m_camera.type = Camera::CameraType::lookat;
	m_camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
	m_camera.setRotation(glm::vec3(0.0f));
	m_camera.setPerspective(60.0f, (float)width / (float)height, 1.0f, 256.0f);
}

ApplicationBase::~ApplicationBase()
{
	m_swapChain.CleanUp();
	if (m_descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	}
	DestroyCommandBuffers();
	if (m_renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	}
	for (auto& frameBuffer : m_frameBuffers) {
		vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
	}
	for (auto& shaderModule : m_shaderModules) {
		vkDestroyShaderModule(m_device, shaderModule, nullptr);
	}
	vkDestroyImageView(m_device, m_depthStencil.view, nullptr);
	vkDestroyImage(m_device, m_depthStencil.image, nullptr);
	vkFreeMemory(m_device, m_depthStencil.memory, nullptr);
	vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr);
	vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
	for (auto& fence : m_waitFences) {
		vkDestroyFence(m_device, fence, nullptr);
	}
	for (auto& semaphore : m_presentCompleteSemaphores) {
		vkDestroySemaphore(m_device, semaphore, nullptr);
	}
	for (auto& semaphore : m_renderCompleteSemaphores) {
		vkDestroySemaphore(m_device, semaphore, nullptr);
	}
	delete vulkanDevice;
	if (debugVulkan) {
		Render::Vulkan::Debug::FreeDebugCallback(m_instance);
	}
	vkDestroyInstance(m_instance, nullptr);
}

#if defined(_WIN32)

void ApplicationBase::SetupConsole(std::string title)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);
	// Enable flags so we can color the output
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(consoleHandle, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(consoleHandle, dwMode);
	SetConsoleTitle(TEXT(title.c_str()));
}

HWND ApplicationBase::SetUpWindow(HINSTANCE instance, WNDPROC wndProc)
{
	this->windowInstance = instance;

	WNDCLASSEX wndClass{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = wndProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = instance,
		.hIcon = LoadIcon(NULL, IDI_APPLICATION),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
		.lpszMenuName = NULL,
		.lpszClassName = name.c_str(),
		.hIconSm = LoadIcon(NULL, IDI_WINLOGO),
	};

	if (!RegisterClassEx(&wndClass))
	{
		std::cout << "Could not register window class!\n";
		fflush(stdout);
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (useFullScreen)
	{
		if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = width;
			dmScreenSettings.dmPelsHeight = height;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					useFullScreen = false;
				}
				else
				{
					return nullptr;
				}
			}
			screenWidth = width;
			screenHeight = height;
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (useFullScreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect{
		.left = 0L,
		.top = 0L,
		.right = useFullScreen ? (long)screenWidth : (long)width,
		.bottom = useFullScreen ? (long)screenHeight : (long)height
	};
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = GetWindowTitle();
	window = CreateWindowEx(0,
		name.c_str(),
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		instance,
		NULL);

	if (!window)
	{
		std::cerr << "Could not create window!\n";
		fflush(stdout);
		return nullptr;
	}

	if (!useFullScreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

	return window;

}

void ApplicationBase::HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		prepared = false;
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		ValidateRect(window, NULL);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}

		break;
	case WM_KEYUP:
		break;
	case WM_LBUTTONDOWN:
		mouseState.position = glm::vec2(float(LOWORD(lParam)), float(HIWORD(lParam)));
		mouseState.button.left = true;
		break;
	case WM_RBUTTONDOWN:
		mouseState.position = glm::vec2(float(LOWORD(lParam)), float(HIWORD(lParam)));
		mouseState.button.right = true;
		break;
	case WM_MBUTTONDOWN:
		mouseState.position = glm::vec2(float(LOWORD(lParam)), float(HIWORD(lParam)));
		mouseState.button.middle = true;
		break;
	case WM_LBUTTONUP:
		mouseState.button.left = false;
		break;
	case WM_RBUTTONUP:
		mouseState.button.right = false;
		break;
	case WM_MBUTTONUP:
		mouseState.button.middle = false;
		break;
	case WM_MOUSEWHEEL:
	{
		break;
	}
	case WM_MOUSEMOVE:
	{
		break;
	}
	case WM_SIZE:
	{
		destWidth = LOWORD(lParam);
		destHeight = HIWORD(lParam);
		if (!prepared)
			return;
		WindowResize();
	}
		break;

	case WM_ENTERSIZEMOVE:
		break;
	case WM_EXITSIZEMOVE:
		break;
	}
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)

int32_t  ApplicationBase::HandleAppInput(struct android_app* app, AInputEvent* event)
{
    ApplicationBase* pBase = reinterpret_cast<ApplicationBase*>(app->userData);
    if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        int32_t eventSource = AInputEvent_getSource(event);
        switch(eventSource)
        {
            case AINPUT_SOURCE_JOYSTICK:
            {
            }
            break;
            case AINPUT_SOURCE_TOUCHSCREEN:
            {
                int32_t action = AMotionEvent_getAction(event);

                switch (action)
                {

                }
            }
            break;
            default:
                return 1;
                break;
        }
    }

    return 0;
}

void ApplicationBase::HandleAppCommand(android_app * app, int32_t cmd)
{
    assert(app->userData != nullptr);
    ApplicationBase* p_appBase = reinterpret_cast<ApplicationBase*>(app->userData);
    switch (cmd)
    {
        case APP_CMD_SAVE_STATE:
            LOGD("APP_CMD_SAVE_STATE");
            /*
            vulkanExample->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)vulkanExample->app->savedState) = vulkanExample->state;
            vulkanExample->app->savedStateSize = sizeof(struct saved_state);
            */
            break;
        case APP_CMD_INIT_WINDOW:
            LOGD("APP_CMD_INIT_WINDOW");
            if (androidApp->window != nullptr)
            {
                if (p_appBase->InitVulkan())
                {
                    p_appBase->Prepare();
                    assert(p_appBase->prepared);
                }
                else {
                    LOGE("Could not initialize Vulkan, exiting!");
                    androidApp->destroyRequested = 1;
                }
            }
            else
            {
                LOGE("No window assigned!");
            }
            break;
        case APP_CMD_LOST_FOCUS:
            LOGD("APP_CMD_LOST_FOCUS");
            p_appBase->focused = false;
            break;
        case APP_CMD_GAINED_FOCUS:
            LOGD("APP_CMD_GAINED_FOCUS");
            p_appBase->focused = true;
            break;
        case APP_CMD_TERM_WINDOW:
            // Window is hidden or closed, clean up resources
            LOGD("APP_CMD_TERM_WINDOW");
            if (p_appBase->prepared) {
                p_appBase->m_swapChain.CleanUp();
            }
            break;
    }
}

#elif defined(VK_USE_PLATFORM_METAL_EXT)
@interface AppDelegate : NSObject<NSApplicationDelegate>
{
@public
    ApplicationBase* theApp;
}

@end


@implementation AppDelegate
{
}

dispatch_group_t concurrentGroup;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [NSApp activateIgnoringOtherApps:YES];
    
    concurrentGroup = dispatch_group_create();
    dispatch_queue_t concurrentQueue = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);
    dispatch_group_async(concurrentGroup, concurrentQueue, ^{

        while (!theApp->quit) {
            theApp->DisplayLinkOutputCb();
        }
    });

    // SRS - When benchmarking, set up termination notification on main thread when concurrent queue completes
    if (true) {
        dispatch_queue_t notifyQueue = dispatch_get_main_queue();
        dispatch_group_notify(concurrentGroup, notifyQueue, ^{ [NSApp terminate:nil]; });
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

// SRS - Tell rendering loop to quit, then wait for concurrent queue to terminate before deleting vulkanExample
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    theApp->quit = YES;
    dispatch_group_wait(concurrentGroup, DISPATCH_TIME_FOREVER);
    vkDeviceWaitIdle(theApp->vulkanDevice->logicalDevice);
    delete(theApp);
}

@end

/*
dispatch_group_t concurrentGroup;
- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [NSApp activateIgnoringOtherApps:YES];
    
    concurrentGroup = dispatch_group_create();
    dispatch_queue_t concurrentQueue = dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0);
    dispatch_group_async(concurrentGroup, concurrentQueue, ^{

        while (!theApp->quit) {
            theApp->DisplayLinkOutputCb();
        }
    });

    // SRS - When benchmarking, set up termination notification on main thread when concurrent queue completes
    if (true) {
        dispatch_queue_t notifyQueue = dispatch_get_main_queue();
        dispatch_group_notify(concurrentGroup, notifyQueue, ^{ [NSApp terminate:nil]; });
    }
    
}
*/

static CVReturn displayLinkOutputCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow,
    const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn, CVOptionFlags *flagsOut,
    void *displayLinkContext)
{
    @autoreleasepool
    {
        auto appBase = static_cast<ApplicationBase*>(displayLinkContext);
        appBase->DisplayLinkOutputCb();
    }
    return kCVReturnSuccess;
}

@interface View : NSView<NSWindowDelegate>
{
@public
    ApplicationBase *vulkanExample;
}

@end

@implementation View
{
    CVDisplayLinkRef displayLink;
}

- (instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:(frameRect)];
    if (self)
    {
        self.wantsLayer = YES;
        self.layer = [CAMetalLayer layer];
    }
    return self;
}

- (void)viewDidMoveToWindow
{
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    // SRS - Disable displayLink vsync rendering in favour of max frame rate concurrent rendering
    //     - vsync command line option (-vs) on macOS now works like other platforms (using VK_PRESENT_MODE_FIFO_KHR)
    //CVDisplayLinkSetOutputCallback(displayLink, &displayLinkOutputCallback, vulkanExample);
    CVDisplayLinkStart(displayLink);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)keyDown:(NSEvent*)event
{
    switch (event.keyCode)
    {
        
        default:
            break;
    }
}

- (void)keyUp:(NSEvent*)event
{
    switch (event.keyCode)
    {
        default:
            break;
    }
}

- (NSPoint)getMouseLocalPoint:(NSEvent*)event
{
    NSPoint location = [event locationInWindow];
    NSPoint point = [self convertPoint:location fromView:nil];
    point.y = self.frame.size.height - point.y;
    return point;
}

- (void)mouseDown:(NSEvent *)event
{
}

- (void)mouseUp:(NSEvent *)event
{
}

- (void)rightMouseDown:(NSEvent *)event
{
}

- (void)rightMouseUp:(NSEvent *)event
{
}

- (void)otherMouseDown:(NSEvent *)event
{

}

- (void)otherMouseUp:(NSEvent *)event
{
}

- (void)mouseDragged:(NSEvent *)event
{

}

- (void)rightMouseDragged:(NSEvent *)event
{

}

- (void)otherMouseDragged:(NSEvent *)event
{

}

- (void)mouseMoved:(NSEvent *)event
{

}

- (void)scrollWheel:(NSEvent *)event
{

}

- (void)windowWillEnterFullScreen:(NSNotification *)notification
{
    
}

- (void)windowWillExitFullScreen:(NSNotification *)notification
{
  
}

- (BOOL)windowShouldClose:(NSWindow *)sender
{
    return TRUE;
}

- (void)windowWillClose:(NSNotification *)notification
{
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
}

@end
 
void* ApplicationBase::SetUpWindow(void* view)
{
#if defined(VK_EXAMPLE_XCODE_GENERATED)
    NSApp = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    auto nsAppDelegate = [AppDelegate new];
    nsAppDelegate->theApp = this;
    [NSApp setDelegate:nsAppDelegate];
    
    const auto kContentRect = NSMakeRect(0.0f, 0.0f, width, height);
    const auto kWindowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
    auto window = [[NSWindow alloc] initWithContentRect:kContentRect
                                              styleMask:kWindowStyle
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
    [window setTitle:@(title.c_str())];
    [window setAcceptsMouseMovedEvents:YES];
    [window center];
    [window makeKeyAndOrderFront:nil];
    if (useFullScreen) {
        [window toggleFullScreen:nil];
    }

    auto nsView = [[View alloc] initWithFrame:kContentRect];
    nsView->vulkanExample = this;
    [window setDelegate:nsView];
    [window setContentView:nsView];
    this->view = (__bridge void*)nsView;
    this->metalLayer = (CAMetalLayer*)nsView.layer;
    
#endif
    
    return view;
}

void ApplicationBase::DisplayLinkOutputCb()
{
    if(prepared)
        NextFrame();
}

void ApplicationBase::MouseDragged(float x, float y)
{
    
}

void ApplicationBase::WindowWillResize(float x, float y)
{
    if (prepared)
    {
        destWidth = x;
        destHeight = y;
        WindowResize();
    }
}

#endif

