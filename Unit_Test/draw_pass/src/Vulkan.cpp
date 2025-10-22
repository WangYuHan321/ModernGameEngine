#include "Vulkan.h"

void RHIVulkan::InitVulkan()
{
	VkResult result = CreateInstance();


}

VkResult RHIVulkan::CreateInstance()
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

}