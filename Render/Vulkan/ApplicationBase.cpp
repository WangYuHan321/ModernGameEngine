#include "ApplicationBase.h"

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
	}

	if (debugVulkan)
	{
		Render::Vulkan::Debug::SetupDebugging(m_instance);
	}

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
	// Defaults to the first device unless specified by command line
	uint32_t selectedDevice = 0;

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
	VkAttachmentReference depthReference{ .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };

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

void ApplicationBase::NextFrame()
{
	Render();
	frameCounter++;
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
	prepared = true;
}

void ApplicationBase::RenderLoop()
{
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
#endif
}

ApplicationBase::ApplicationBase()
{
#if defined(_WIN32)
	if (debugVulkan)
	{
		SetupConsole("Debug Console");
	}
#endif
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
		}

		break;
	case WM_KEYUP:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_MBUTTONDOWN:

		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONUP:
		break;
	case WM_MBUTTONUP:
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
		break;

	case WM_ENTERSIZEMOVE:
		break;
	case WM_EXITSIZEMOVE:
		break;
	}
}

#endif