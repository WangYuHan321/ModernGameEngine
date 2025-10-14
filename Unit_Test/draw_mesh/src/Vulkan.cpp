#include "Vulkan.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define SHADOW_DIM 1024

// 接收调试信息的回调函数：必须加上那两个宏定义，才能确保被Vulkan库调用
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage
        << std::endl;
    return VK_FALSE;
}

// 把有关回调函数信使的初始化结构体提取出来
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

/** 将编译的着色器二进制SPV文件，读入内存Buffer中*/
static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkFormat RHIVulkan::FindDepthFormat()
{
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

RHIVulkan::RHIVulkan()
{
}

RHIVulkan::~RHIVulkan()
{
}

/** 找到物理硬件支持的图片格式*/
VkFormat RHIVulkan::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void RHIVulkan::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_glfwWindow = glfwCreateWindow(1024, 1080, "我的第一个程序", nullptr, nullptr);
}


void RHIVulkan::InitVulkan()
{
    CreateInstance();//创建Vulkan实例
    SetupDebugMessage();//设置debug
    CreateSurface();//创建Window
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    //CreateDepthResource();
    CreateRenderPass();
    CreateUniformBuffer();
    CreateFrameBuffer();
    CreateCommandPool();
    CreateCommandBuffers();

    CreateShadowmapPass();
    CreateBaseScenePass();
    //CreateGraphicsPipeline();

    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateSyncObjects();
}

void RHIVulkan::MainLoop()
{
    while (!glfwWindowShouldClose(m_glfwWindow)) {
        glfwPollEvents();
        DrawFrame();
    }
}

//获取GLFW所有扩展
std::vector<const char*> RHIVulkan::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

//检查device是否支持extension
bool RHIVulkan::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

//设置验证层Message
void RHIVulkan::PopulateDebugMessagerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

//检查所有的请求验证层是否可用
bool RHIVulkan::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {//VK_LAYER_LUNARG_standard_validation
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

RHIVulkan::SwapChainSupportDetails RHIVulkan::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surfaceWindow, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surfaceWindow, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surfaceWindow, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surfaceWindow, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surfaceWindow, &presentModeCount, details.presentModes.data());
    }

    return details;
}

RHIVulkan::QueueFamilyIndices RHIVulkan::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;//支持图形命令队列
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surfaceWindow, &presentSupport);//支持呈现显示队列

        if (queueFamily.queueCount > 0 && presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;

}

VkPresentModeKHR RHIVulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RHIVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        VkExtent2D actualExtent = { m_windowWidth, m_windowHeight };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkSurfaceFormatKHR RHIVulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

bool RHIVulkan::IsDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VkShaderModule RHIVulkan::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());


    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_vkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;

}

void RHIVulkan::CreateInstance()
{

    if (enableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested , but not avaliable!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Game Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

#ifdef __APPLE__
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    /** Get Required Extensions*/
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#ifdef __APPLE__
    // Fix issue on Mac(m2) "vkCreateInstance: Found no drivers!"
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    // Issue on Mac(m2), it's not a error, but a warning, ignore it by this time~
    // vkCreateDevice():  VK_KHR_portability_subset must be enabled because physical device VkPhysicalDevice 0x600003764f40[] supports it. The Vulkan spec states: If the VK_KHR_portability_subset extension is included in pProperties of vkEnumerateDeviceExtensionProperties, ppEnabledExtensionNames must include "VK_KHR_portability_subset"
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void RHIVulkan::SetupDebugMessage()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessagerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void RHIVulkan::CreateSurface()
{
    if (glfwCreateWindowSurface(m_vkInstance, m_glfwWindow, nullptr, &m_surfaceWindow) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void RHIVulkan::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    /*  VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);
      vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, physical_devices);
      m_physicalDevice = *physical_devices;*/

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void RHIVulkan::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_vkDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_vkDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_vkDevice, indices.presentFamily.value(), 0, &m_presentQueue);

}

void RHIVulkan::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surfaceWindow;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_vkDevice, &createInfo, nullptr, &m_vkSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &imageCount, nullptr);
    m_vkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapChain, &imageCount, m_vkSwapChainImages.data());

    m_vkSwapChainImageFormat = surfaceFormat.format;
    m_vkSwapChainExtent = extent;
}

void RHIVulkan::ReCreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_glfwWindow, &width, &height);
    
    
    vkDeviceWaitIdle(m_vkDevice);
    CleanSwapChain();
    
    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffer();
    
}

void RHIVulkan::CleanSwapChain()
{
    vkDestroyImageView(m_vkDevice, m_depthImageView, nullptr);
    vkDestroyImage(m_vkDevice, m_depthImage, nullptr);
    vkFreeMemory(m_vkDevice, m_depthImageMemory, nullptr);

    for (auto framebuffer : m_vkSwapChainFrameBuffers) {
        vkDestroyFramebuffer(m_vkDevice, framebuffer, nullptr);
    }

    for (auto imageView : m_vkSwapChainImageViews) {
        vkDestroyImageView(m_vkDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, nullptr);
}

void RHIVulkan::CreateImageViews()
{
    m_vkSwapChainImageViews.resize(m_vkSwapChainImages.size());

    
    for(size_t i =0; i< m_vkSwapChainImages.size(); i++)
    {
        m_vkSwapChainImageViews[i] = CreateImageView(m_vkSwapChainImages[i], m_vkSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
    
    /*
    for (uint32_t i = 0; i < m_vkSwapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_vkSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_vkSwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_vkDevice, &createInfo, nullptr, &m_vkSwapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
     
     */
}

static void PrintNodes(const tinygltf::Scene &scene) {
  for (size_t i = 0; i < scene.nodes.size(); i++) {
    std::cout << "node.name : " << scene.nodes[i] << std::endl;
  }
}


void RHIVulkan::ReadModelResource()
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string error;
    std::string warning;
    loader.LoadASCIIFromFile(&model, &error, &warning, "./Asset/mesh/Avocado/Avocado.gltf");
    
    for (const auto& mesh : model.meshes) {
            for (const auto& primitive : mesh.primitives) {
                // --- LOAD VERTEX POSITIONS ---
                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.at("POSITION")];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                    std::cout << "Loading " << accessor.count << " vertices..." << std::endl;
                    for (size_t i = 0; i < accessor.count; i++) {
                        // Positions are 3-component vectors (x, y, z)
                        size_t stride = accessor.ByteStride(bufferView);
                        const float* pos = (const float*)((const char*)positions + i * stride);
                        //std::cout << "  Vertex " << i << ": (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
                    }
                }

                // --- LOAD INDICES ---
                if (primitive.indices >= 0) {
                    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                    std::cout << "Loading " << accessor.count << " indices..." << std::endl;
                    
                    // Get the base pointer to the index data
                    const uint8_t* indexData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

                    for (size_t i = 0; i < accessor.count; i++) {
                        int index = -1;
                        // Determine the index data type
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            index = static_cast<int>(reinterpret_cast<const uint8_t*>(indexData)[i]);
                        } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            index = static_cast<int>(reinterpret_cast<const uint16_t*>(indexData)[i]);
                        } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                            index = static_cast<int>(reinterpret_cast<const uint32_t*>(indexData)[i]);
                        }

                        if (index != -1) {
                           // std::cout << "  Index " << i << ": " << index << std::endl;
                        }
                    }
                }
            }
        }
}

void RHIVulkan::CreateUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_vkUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0;i < MAX_FRAMES_IN_FLIGHT;i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vkUniformBuffers[i], m_vkUniformBuffersMemory[i]);
    
        // 这里会导致 memory stack overflow，不应该在这里 vkMapMemory
        //vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }

    bufferSize = sizeof(UniformBufferView);

    m_vkViewUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkViewUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vkViewUniformBuffers[i], m_vkViewUniformBuffersMemory[i]);

        // 这里会导致 memory stack overflow，不应该在这里 vkMapMemory
        //vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void RHIVulkan::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_vkDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vkDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_vkDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_vkDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_vkUniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        // 写入 Textures
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textureImageView;
        imageInfo.sampler = m_textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_vkDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;                ;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;//

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_vkDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void RHIVulkan::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_vkDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void RHIVulkan::CreateDepthResource()
{
    VkFormat depthFormat = FindDepthFormat();
    
    CreateImage(m_vkSwapChainExtent.width, m_vkSwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
    m_depthImageView = CreateImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void RHIVulkan::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_vkSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // 渲染子通道 SubPass
    // SubPass是RenderPass的下属任务，和RenderPass共享Framebuffer等渲染资源
    // 某些渲染操作，比如后处理的Blooming，当前渲染需要依赖上一个渲染结果，但是渲染资源不变，这是SubPass可以优化性能
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // 这里将渲染三角形的操作，简化成一个SubPass提交
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_vkRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RHIVulkan::CreateGraphicsPipeline()
{
    auto vertShaderCode = readFile("./Asset/shader/glsl/draw_mesh/draw_mesh_vert.spv");
    auto fragShaderCode = readFile("./Asset/shader/glsl/draw_mesh/draw_mesh_frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_vkSwapChainExtent.width;
    viewport.height = (float)m_vkSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_vkSwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
#ifndef __APPLE__
    rasterizer.depthClampEnable = VK_TRUE;
#endif
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_FALSE;// VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    
    VkPipelineDepthStencilStateCreateInfo depthSencilAttachment;
    depthSencilAttachment.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthSencilAttachment.depthTestEnable = VK_TRUE;
    depthSencilAttachment.depthWriteEnable = VK_TRUE;
    depthSencilAttachment.depthCompareOp = VK_COMPARE_OP_LESS;
    depthSencilAttachment.depthBoundsTestEnable = VK_FALSE;
    depthSencilAttachment.minDepthBounds = 0.0f; // Optional
    depthSencilAttachment.maxDepthBounds = 1.0f; // Optional
    depthSencilAttachment.stencilTestEnable = VK_FALSE; // 没有写轮廓信息，所以跳过轮廓测试
    depthSencilAttachment.flags = 0;
    depthSencilAttachment.pNext = nullptr;
    depthSencilAttachment.front = {}; // Optional
    depthSencilAttachment.back = {}; // Optional
    
    VkPipelineDynamicStateCreateInfo dynamicStateCI{};
    dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCI.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_vkDescriptorSetLayout;

    if (vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_vkPipeLineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = &depthSencilAttachment;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateCI;
    pipelineInfo.layout = m_vkPipeLineLayout;
    pipelineInfo.renderPass = m_vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_vkDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_vkDevice, vertShaderModule, nullptr);
}

void RHIVulkan::CreateFrameBuffer()
{
    CreateDepthResource();
    m_vkSwapChainFrameBuffers.resize(m_vkSwapChainImageViews.size());

    for (size_t i = 0; i < m_vkSwapChainImageViews.size(); i++)
    {
        std::array<VkImageView, 2> attachments = {
            m_vkSwapChainImageViews[i],
            m_depthImageView
        };

        VkFramebufferCreateInfo frameBufferInfo{};

        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_vkRenderPass;
        frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        frameBufferInfo.pAttachments = attachments.data();
        frameBufferInfo.width = m_vkSwapChainExtent.width;
        frameBufferInfo.height = m_vkSwapChainExtent.height;
        frameBufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_vkDevice, &frameBufferInfo, nullptr, &m_vkSwapChainFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

}

void RHIVulkan::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

    if (vkCreateCommandPool(m_vkDevice, &poolInfo, nullptr, &m_vkCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void RHIVulkan::CreateGeometry(std::vector<RenderObject>& outRenderObj, const std::string& fileName)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string error;
    std::string warning;
    loader.LoadASCIIFromFile(&model, &error, &warning, fileName);

    //texture model.textures
    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            std::vector<Vertex> vertex = {};
            std::vector<uint32_t> vertexIndex = {};
            // --- LOAD VERTEX POSITIONS ---
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const tinygltf::Accessor& accessorPos = model.accessors[primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& bufferViewPos = model.bufferViews[accessorPos.bufferView];
                const tinygltf::Buffer& bufferPos = model.buffers[bufferViewPos.buffer];

                const float* positions = reinterpret_cast<const float*>(&bufferPos.data[bufferViewPos.byteOffset + accessorPos.byteOffset]);

                std::cout << "Loading " << accessorPos.count << " vertices..." << std::endl;

                if (vertex.size() == 0) vertex.resize(accessorPos.count);
                for (size_t i = 0; i < accessorPos.count; i++) {
                    // Positions are 3-component vectors (x, y, z)
                    size_t stride = accessorPos.ByteStride(bufferViewPos);
                    const float* pos = (const float*)((const char*)positions + i * stride);
					vertex[i].Position = {pos[0], pos[1], pos[2]};
                }
            }

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const tinygltf::Accessor& accessorNor = model.accessors[primitive.attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferViewNor = model.bufferViews[accessorNor.bufferView];
                const tinygltf::Buffer& bufferNor = model.buffers[bufferViewNor.buffer];

                const float* normals = reinterpret_cast<const float*>(&bufferNor.data[bufferViewNor.byteOffset + accessorNor.byteOffset]);

                std::cout << "Loading " << accessorNor.count << " normals..." << std::endl;
                for (size_t i = 0; i < accessorNor.count; i++) {
                    // Positions are 3-component vectors (x, y, z)
                    size_t stride = accessorNor.ByteStride(bufferViewNor);
                    const float* pos = (const float*)((const char*)normals + i * stride);
					vertex[i].Normal = {pos[0], pos[1], pos[2]};
                }
            }

            if (primitive.attributes.find("COLOR") != primitive.attributes.end()) {
                const tinygltf::Accessor& accessorNor = model.accessors[primitive.attributes.at("COLOR")];
                const tinygltf::BufferView& bufferViewNor = model.bufferViews[accessorNor.bufferView];
                const tinygltf::Buffer& bufferNor = model.buffers[bufferViewNor.buffer];

                const float* normals = reinterpret_cast<const float*>(&bufferNor.data[bufferViewNor.byteOffset + accessorNor.byteOffset]);

                std::cout << "Loading " << accessorNor.count << " colors..." << std::endl;
                for (size_t i = 0; i < accessorNor.count; i++) {
                    // Positions are 3-component vectors (x, y, z)
                    size_t stride = accessorNor.ByteStride(bufferViewNor);
                    const float* pos = (const float*)((const char*)normals + i * stride);
                    vertex[i].Color = {pos[0], pos[1], pos[2]};
                }
            }

            // --- LOAD INDICES ---
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                std::cout << "Loading " << accessor.count << " indices..." << std::endl;

                // Get the base pointer to the index data
                const uint8_t* indexData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

                if (vertexIndex.size() == 0) vertexIndex.resize(accessor.count);

                for (size_t i = 0; i < accessor.count; i++) {
                    int index = -1;
                    // Determine the index data type
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        index = static_cast<int>(reinterpret_cast<const uint8_t*>(indexData)[i]);
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        index = static_cast<int>(reinterpret_cast<const uint16_t*>(indexData)[i]);
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        index = static_cast<int>(reinterpret_cast<const uint32_t*>(indexData)[i]);
                    }

                    if (index != -1) {
                        vertexIndex[i] = index;
                    }
                }
            }
            RenderObject renderObjectTemp;
            renderObjectTemp.meshData.Vertices = vertex;
            renderObjectTemp.meshData.Indices = vertexIndex;
            outRenderObj.push_back(renderObjectTemp);
        }
    }
}

void RHIVulkan::CreateShadowmapPass()
{
    m_shadowmapPass.Width = SHADOW_DIM;
	m_shadowmapPass.Height = SHADOW_DIM;
	m_shadowmapPass.Format = FindDepthFormat();
    CreateImage(m_shadowmapPass.Width, m_shadowmapPass.Height, m_shadowmapPass.Format, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,m_shadowmapPass.Image, m_shadowmapPass.Memory);
    m_shadowmapPass.ImageView = CreateImageView(m_shadowmapPass.Image, m_shadowmapPass.Format, VK_IMAGE_ASPECT_DEPTH_BIT);
    CreateSampler(m_shadowmapPass.Sampler,
        VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

	// create uniform buffer for shadowmap
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    m_shadowmapPass.UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_shadowmapPass.UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    for(size_t i= 0;i<MAX_FRAMES_IN_FLIGHT;i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_shadowmapPass.UniformBuffers[i], m_shadowmapPass.UniformBuffersMemory[i]);
	}

    //create descriptorsetlayout
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_shadowmapPass.DescriptionSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

	//create descriptorpool
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    if (vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &m_shadowmapPass.DescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
	}

	//create descriptorsets
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_shadowmapPass.DescriptionSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_shadowmapPass.DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();
    m_shadowmapPass.DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, m_shadowmapPass.DescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // binding desc
    for (size_t i = 0;i < MAX_FRAMES_IN_FLIGHT;i++)
    {
        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
		VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_shadowmapPass.UniformBuffers[i];
		bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_shadowmapPass.DescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
        vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    VkAttachmentDescription attachmentDescription{};
	attachmentDescription.format = m_shadowmapPass.Format;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthReference{};
	depthReference.attachment = 0;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPass{};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 0;
    subPass.pDepthStencilAttachment = &depthReference;

	//use subpass dependency for layout transition
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    //  srcStageMask 表示同步的起始管线阶段
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //  dstStageMask表示同步的终止管线阶段。
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    //  着色器对附件以外的读取操作 源访问操作
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //  对深度模板附件的写入操作 目标访问操作
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subPass;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassCreateInfo.pDependencies = dependencies.data();


    if (vkCreateRenderPass(m_vkDevice, &renderPassCreateInfo, nullptr, &m_shadowmapPass.RenderPass))
    {
        throw std::runtime_error("failed to create shadow map render pass !");
    }

    //create frame buffer
    VkFramebufferCreateInfo bufCreateInfo{};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    bufCreateInfo.renderPass = m_shadowmapPass.RenderPass;
    bufCreateInfo.attachmentCount = 1;
	bufCreateInfo.pAttachments = &m_shadowmapPass.ImageView;
    bufCreateInfo.width = m_shadowmapPass.Width;
    bufCreateInfo.height = m_shadowmapPass.Height;
    bufCreateInfo.layers = 1;

    if (vkCreateFramebuffer(m_vkDevice, &bufCreateInfo, nullptr, &m_shadowmapPass.FrameBuffer))
    {
        throw std::runtime_error("failed to create shadow map frame buffer !");
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
    inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;
    VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
    rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCI.depthClampEnable = VK_FALSE;
    rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCI.lineWidth = 1.0f;
    rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCI.depthBiasEnable = VK_FALSE;
    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.colorWriteMask = 0xf;
    blendAttachmentState.blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttachmentState;
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;
    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = {};
    depthStencilState.back = {};
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    VkPipelineMultisampleStateCreateInfo multiSampleState{};
    multiSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampleState.sampleShadingEnable = VK_FALSE;
    multiSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    auto vertShaderCode = readFile("./Asset/shader/glsl/draw_mesh/draw_mesh_shadow_vert.spv");
    auto fragShaderCode = readFile("./Asset/shader/glsl/draw_mesh/draw_mesh_shadow_frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo , fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    auto bindingDesc = Vertex::GetBindingDescription();
    auto attrDesc = Vertex::GetAttributeDescriptions();
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
    vertexInput.pVertexBindingDescriptions = &bindingDesc;
    vertexInput.pVertexAttributeDescriptions = attrDesc.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_shadowmapPass.DescriptionSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_shadowmapPass.PipelineLayout))
    {
        throw std::runtime_error("failed to create pipelinelayout !");
    }

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStages;
    pipelineCI.pVertexInputState = &vertexInput;
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizationStateCI;
    pipelineCI.pMultisampleState = &multiSampleState;
    pipelineCI.pDepthStencilState = &depthStencilState; // 加上深度测试
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = m_shadowmapPass.PipelineLayout;
    pipelineCI.renderPass = m_vkRenderPass;
    pipelineCI.subpass = 0;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

    // Offscreen pipeline (vertex shader only)
    //shaderStages[0] = loadShader(getShadersPath() + "shadowmapping/offscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipelineCI.stageCount = 1;
    // No blend attachment states (no color attachments used)
    colorBlendState.attachmentCount = 0;
    // Disable culling, so all faces contribute to shadows
    rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    // Enable depth bias
    rasterizationStateCI.depthBiasEnable = VK_TRUE;
    // Add depth bias to dynamic state, so we can change it at runtime
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    pipelineCI.renderPass = m_shadowmapPass.RenderPass;

    if (vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_shadowmapPass.Pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_vkDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_vkDevice, vertShaderModule, nullptr);
}

void RHIVulkan::CreateBaseScenePass()
{
    CreateDescriptorSetLayout(m_baseScenePass.DescriptorSetLayout, 3);

    m_baseScenePass.Pipelines.resize(1);
    CreateGraphicsPipeline(m_baseScenePass.PipelineLayout,
        m_baseScenePass.Pipelines, 1,
        m_baseScenePass.DescriptorSetLayout,
        "./Asset/shader/glsl/draw_mesh/draw_mesh_base_vert.spv",
        "./Asset/shader/glsl/draw_mesh/draw_mesh_base_frag.spv");

    CreateGeometry(m_baseScenePass.RenderObjects, "./Asset/mesh/Avocado/Avocado.gltf");
    std::vector<std::string> pngFiles;
    pngFiles.push_back("./Asset/mesh/Avocado/Avocado_baseColor.png");
    pngFiles.push_back("./Asset/mesh/Avocado/Avocado_normal.png");
    pngFiles.push_back("./Asset/mesh/Avocado/Avocado_roughnessMetallic.png");

    m_baseScenePass.RenderObjects[0].mat.TextureImageViews.resize(pngFiles.size());
    m_baseScenePass.RenderObjects[0].mat.TextureSamplers.resize(pngFiles.size());
    m_baseScenePass.RenderObjects[0].mat.TextureImages.resize(pngFiles.size());
    m_baseScenePass.RenderObjects[0].mat.TextureImageMemorys.resize(pngFiles.size());
    m_baseScenePass.RenderObjects[0].mat.TextureSamplers.resize(pngFiles.size());

    for (int i = 0;i < pngFiles.size(); i++)
    {
        CreateImageContext(m_baseScenePass.RenderObjects[0].mat.TextureImages[i], m_baseScenePass.RenderObjects[0].mat.TextureImageMemorys[0],
            m_baseScenePass.RenderObjects[0].mat.TextureImageViews[i], m_baseScenePass.RenderObjects[0].mat.TextureSamplers[i], pngFiles[i], true);
    }

    int i = 0;
    for (auto& item : m_baseScenePass.RenderObjects)
    {
        CreateVertexBuffer(item.meshData.VertexBuffer, item.meshData.VertexBufferMemory, item.meshData.Vertices);
        CreateIndexBuffer(item.meshData.IndexBuffer, item.meshData.IndexBufferMemory, item.meshData.Indices);
        CreateDescriptorPool(item.mat.DescriptorPool, 4);
        CreateDescriptorSets(item.mat.DescriptorSets, item.mat.DescriptorPool, m_baseScenePass.DescriptorSetLayout, item.mat.TextureImageViews, item.mat.TextureSamplers);
        i++;
    }


}

static stbi_uc* readTextureAsset(const std::string& filename, int& texWidth, int& texHeight, int& texChannels, int& mipLevels)
{
    stbi_hdr_to_ldr_scale(2.2f);
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    stbi_hdr_to_ldr_scale(1.0f);
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
        assert(true);
    }
    return pixels;
}

void RHIVulkan::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    // 检查图像格式是否支持 linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    EndSingleTimeCommandBuffer(commandBuffer);
}

void RHIVulkan::CreateImage(
    VkImage& outImage,
    VkDeviceMemory& OutImageMemory,
    const uint32_t width, const uint32_t height, const VkFormat format,
    const VkImageTiling tiling, const VkImageUsageFlags usage,
    const VkMemoryPropertyFlags properties, const uint32_t miplevels)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = miplevels;

    if (vkCreateImage(m_vkDevice, &imageInfo, nullptr, &outImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_vkDevice, outImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &OutImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_vkDevice, outImage, OutImageMemory, 0);
}

void RHIVulkan::CreateImageView(
    VkImageView& outImageView,
    const VkImage& inImage,
    const VkFormat inFormat,
    const VkImageAspectFlags aspectFlags,
    const uint32_t levelCount)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = inImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = inFormat;
    viewInfo.subresourceRange.aspectMask = aspectFlags; // VK_IMAGE_ASPECT_COLOR_BIT 颜色 VK_IMAGE_ASPECT_DEPTH_BIT 深度
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_vkDevice, &viewInfo, nullptr, &outImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void RHIVulkan::CreateImageContext(
    VkImage& outImage,
    VkDeviceMemory& outMemory,
    VkImageView& outImageView,
    VkSampler& outSampler,
    const std::string& filename, bool sRGB)
{
    int texWidth, texHeight, texChannels, mipLevels;
    stbi_uc* pixels = readTextureAsset(filename, texWidth, texHeight, texChannels, mipLevels);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    // 清理pixels数据结构
    stbi_image_free(pixels);

    VkFormat format = sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
    // VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT 告诉Vulkan这张贴图即要被读也要被写
    CreateImage(
        outImage,
        outMemory,
        texWidth, texHeight, format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipLevels);

    TransitionImageLayout(outImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    CopyBufferToImage(stagingBuffer, outImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    //transitionImageLayout(outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);

    GenerateMipmaps(outImage, format, texWidth, texHeight, mipLevels);

    CreateImageView(outImageView, outImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
    CreateSampler(outSampler,
        VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        mipLevels);
}

void RHIVulkan::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t miplevels)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = miplevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommandBuffer(commandBuffer);
}

void RHIVulkan::CreateDescriptorSets(std::vector<VkDescriptorSet>& outDescriptorSets, const VkDescriptorPool& inDescriptorPool, const VkDescriptorSetLayout& inDescriptorSetLayout, const std::vector<VkImageView>& inImageViews, const std::vector<VkSampler>& inSamplers)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, inDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = inDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    outDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_vkDevice, &allocInfo, outDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        uint32_t write_size = static_cast<uint32_t>(inImageViews.size()) + 3; // 这里加2为 UniformBuffer 的个数
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(write_size);

        // 绑定 UnifromBuffer
        VkDescriptorBufferInfo baseBufferInfo{};
        baseBufferInfo.buffer = m_vkUniformBuffers[i];
        baseBufferInfo.offset = 0;
        baseBufferInfo.range = sizeof(UniformBufferObject);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = outDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &baseBufferInfo;

        // 绑定 UnifromBuffer
        VkDescriptorBufferInfo viewBufferInfo{};
        viewBufferInfo.buffer = m_vkViewUniformBuffers[i];
        viewBufferInfo.offset = 0;
        viewBufferInfo.range = sizeof(UniformBufferView);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = outDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &viewBufferInfo;


        VkDescriptorImageInfo shadowmapImageInfo{};
        shadowmapImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        shadowmapImageInfo.imageView = m_shadowmapPass.ImageView;
        shadowmapImageInfo.sampler = m_shadowmapPass.Sampler;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = outDescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &shadowmapImageInfo;

        // 绑定 Textures
        // descriptorWrites会引用每一个创建的VkDescriptorImageInfo，所以需要用一个数组把它们存储起来
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.resize(inImageViews.size());
        for (size_t j = 0; j < inImageViews.size(); j++)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = inImageViews[j];
            imageInfo.sampler = inSamplers[j];
            imageInfos[j] = imageInfo;

            descriptorWrites[j + 3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j + 3].dstSet = outDescriptorSets[i];
            descriptorWrites[j + 3].dstBinding = static_cast<uint32_t>(j + 3);
            descriptorWrites[j + 3].dstArrayElement = 0;
            descriptorWrites[j + 3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j + 3].descriptorCount = 1;
            descriptorWrites[j + 3].pImageInfo = &imageInfos[j]; // 注意，这里是引用了VkDescriptorImageInfo，所有需要创建imageInfos这个数组，存储所有的imageInfo而不是使用局部变量imageInfo
        }

        vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void RHIVulkan::CreateDescriptorPool(VkDescriptorPool& outDescriptorPool, uint32_t sampler_num )
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.resize(sampler_num + 2); // 这里3是2个UniformBuffer和一个环境Cubemap贴图
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < sampler_num; i++)
    {
        poolSizes[i + 2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[i + 2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_vkDevice, &poolInfo, nullptr, &outDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void RHIVulkan::CreateDescriptorSetLayout(VkDescriptorSetLayout& outDescriptorSetLayout, uint32_t sampler_number)
{
    // UnifromBufferObject（ubo）绑定
    VkDescriptorSetLayoutBinding baseUBOLayoutBinding{};
    baseUBOLayoutBinding.binding = 0;
    baseUBOLayoutBinding.descriptorCount = 1;
    baseUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    baseUBOLayoutBinding.pImmutableSamplers = nullptr;
    baseUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // UnifromBufferObject（ubo）绑定
    VkDescriptorSetLayoutBinding viewUBOLayoutBinding{};
    viewUBOLayoutBinding.binding = 1;
    viewUBOLayoutBinding.descriptorCount = 1;
    viewUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    viewUBOLayoutBinding.pImmutableSamplers = nullptr;
    viewUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // view ubo 主要信息用于 fragment shader

    VkDescriptorSetLayoutBinding shadowmapLayoutBinding{};
    shadowmapLayoutBinding.binding = 2;
    shadowmapLayoutBinding.descriptorCount = 1;
    shadowmapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowmapLayoutBinding.pImmutableSamplers = nullptr;
    shadowmapLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(sampler_number + 3); 
    bindings[0] = baseUBOLayoutBinding;
    bindings[1] = viewUBOLayoutBinding;
    bindings[2] = shadowmapLayoutBinding;
    for (size_t i = 0; i < sampler_number; i++)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = static_cast<uint32_t>(i + 3);
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[i + 3] = samplerLayoutBinding;
    }
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();


    if (vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &outDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void RHIVulkan::CreateGraphicsPipeline(VkPipelineLayout& outPipelineLayout,
    std::vector<VkPipeline>& outPipelines,
    const uint32_t specConstantsCount,
    const VkDescriptorSetLayout& inDescriptorSetLayout,
    const std::string& vert_filename,
    const std::string& frag_filename,
    const VkBool32 bDepthTest,
    const VkCullModeFlags CullMode)
{
    auto vertShaderCode = readFile(vert_filename);
    auto fragShaderCode = readFile(frag_filename);

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // 顶点缓存绑定的描述，定义了顶点都需要绑定什么数据，比如第一个位置绑定Position，第二个位置绑定Color，第三个位置绑定UV等
    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    // 渲染管线VertexBuffer输入
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    if (bDepthTest == VK_FALSE && CullMode == VK_CULL_MODE_NONE) // 如果不做深度检测并且不做背面剔除，那么认为是渲染背景，不需要绑定VBO
    {
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
    }
    else // 正常VBO渲染绑定
    {
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    // 关闭背面剔除，使得材质TwoSide渲染
    rasterizer.cullMode = CullMode /*VK_CULL_MODE_BACK_BIT*/;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // 打开深度测试
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = bDepthTest; //VK_TRUE;
    depthStencil.depthWriteEnable = bDepthTest; //VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE; // 没有写轮廓信息，所以跳过轮廓测试
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    //// 设置 push constants
    //VkPushConstantRange pushConstant;
    //// 这个PushConstant的范围从头开始
    //pushConstant.offset = 0;
    //pushConstant.size = sizeof(GlobalConstants);
    //// 这是个全局PushConstant，所以希望各个着色器都能访问到
    //pushConstant.stageFlags = VK_SHADER_STAGE_ALL;

    // 在渲染管线创建时，指定DescriptorSetLayout，用来传UniformBuffer
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &inDescriptorSetLayout;
    //pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;

    // PipelineLayout可以用来创建和绑定VertexBuffer和UniformBuffer，这样可以往着色器中传递参数
    if (vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &outPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil; // 加上深度测试
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = outPipelineLayout;
    pipelineInfo.renderPass = m_vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    // Use specialization constants 优化着色器变体
    for (uint32_t i = 0; i < specConstantsCount; i++)
    {
        uint32_t specConstants = i;
        VkSpecializationMapEntry specializationMapEntry = VkSpecializationMapEntry{};
        specializationMapEntry.constantID = 0;
        specializationMapEntry.offset = 0;
        specializationMapEntry.size = sizeof(uint32_t);
        VkSpecializationInfo specializationInfo = VkSpecializationInfo();
        specializationInfo.mapEntryCount = 1;
        specializationInfo.pMapEntries = &specializationMapEntry;
        specializationInfo.dataSize = sizeof(uint32_t);
        specializationInfo.pData = &specConstants;
        shaderStages[1].pSpecializationInfo = &specializationInfo;

        if (vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &outPipelines[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    vkDestroyShaderModule(m_vkDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_vkDevice, vertShaderModule, nullptr);
}

void RHIVulkan::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uniformLayoutBingding{};
    uniformLayoutBingding.binding = 0;
    uniformLayoutBingding.descriptorCount = 1;
    uniformLayoutBingding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformLayoutBingding.pImmutableSamplers = nullptr;
    uniformLayoutBingding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //VK_SHADER_STAGE_VERTEX_BIT

    // 将UnifromBufferObject和贴图采样器绑定到DescriptorSetLayout上
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uniformLayoutBingding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_vkDevice, &layoutInfo, nullptr, &m_vkDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void RHIVulkan::UpdateUniformBuffer(uint32_t currentImage)
{
	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 camView = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraFOV = 45;
    float zNear = 0.1;
    float zFar = 100;

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    m_shadowmapPass.zNear = zNear;
    m_shadowmapPass.zFar = zFar;

    Light light;
    glm::vec3 center = glm::vec3(0.0f);
    glm::vec3 lightPos = glm::vec3(4.0f, 0.0f, 4.0f);
    float rollLight = time * 45.0f;
    lightPos.x = cos(glm::radians(rollLight)) * 4.0f;
    lightPos.y = sin(glm::radians(rollLight)) * 4.0f;
    glm::vec3 lightDir = glm::normalize(lightPos - center);

    light.position = glm::vec4(lightPos, 0.0);
    light.color = glm::vec4(1.0, 1.0, 1.0, 3.0);
    light.direction = glm::vec4(lightDir, 0.0);
    light.info = glm::vec4(0.0, 0.0, 0.0, 0.0);

    glm::mat4 localToWorld = glm::rotate(glm::mat4(1.0f), rollLight, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 shadowView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 shadowProjection = glm::perspective(glm::radians(cameraFOV), 1.0f, m_shadowmapPass.zNear, m_shadowmapPass.zFar);


    UniformBufferObject UBOBaseData{};
    UBOBaseData.Model = localToWorld;
    UBOBaseData.View = glm::lookAt(camPos, camView, camUp);
    UBOBaseData.Proj = glm::perspective(glm::radians(cameraFOV), m_vkSwapChainExtent.width / (float)m_vkSwapChainExtent.height, zNear, zFar);
    UBOBaseData.Proj[1][1] *= -1;

    void* data_base_ubo;
    vkMapMemory(m_vkDevice, m_vkUniformBuffersMemory[currentImage], 0, sizeof(UBOBaseData), 0, &data_base_ubo);
    memcpy(data_base_ubo, &UBOBaseData, sizeof(UBOBaseData));
    vkUnmapMemory(m_vkDevice, m_vkUniformBuffersMemory[currentImage]);

    UniformBufferView UBOViewData{};
    // shadowmap_space 的 MVP 矩阵中，M矩阵在FS中计算，所以传入 localToWorld 进入FS
    UBOViewData.shadowmap_space = shadowProjection * shadowView;
    UBOViewData.local_to_world = localToWorld;
    UBOViewData.camera_info = glm::vec4(camPos, cameraFOV);
    UBOViewData.lights_count = glm::ivec4(1, 0, 0, 2);
    UBOViewData.directional_lights[0] = light;
    UBOViewData.zNear = m_shadowmapPass.zNear;
    UBOViewData.zFar = m_shadowmapPass.zFar;

    void* data_view;
    vkMapMemory(m_vkDevice, m_vkViewUniformBuffersMemory[currentImage], 0, sizeof(UBOViewData), 0, &data_view);
    memcpy(data_view, &UBOViewData, sizeof(UBOViewData));
    vkUnmapMemory(m_vkDevice, m_vkViewUniformBuffersMemory[currentImage]);

    UniformBufferObject UBOShadowData{};
    UBOShadowData.Model = localToWorld;
    UBOShadowData.View = shadowView;
    UBOShadowData.Proj = shadowProjection;

    void* data_shadow_ubo;
    vkMapMemory(m_vkDevice, m_shadowmapPass.UniformBuffersMemory[currentImage], 0, sizeof(UBOShadowData), 0, &data_shadow_ubo);
    memcpy(data_shadow_ubo, &UBOShadowData, sizeof(UBOShadowData));
    vkUnmapMemory(m_vkDevice, m_shadowmapPass.UniformBuffersMemory[currentImage]);

}

VkImageView RHIVulkan::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_vkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void RHIVulkan::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_vkDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_vkDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_vkDevice, image, imageMemory, 0);
}

void RHIVulkan::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    unsigned char *pixels = stbi_load("./Asset/texture/CesiumLogoFlat.png", &texWidth, &texHeight, 0, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    // 清理pixels数据结构
    stbi_image_free(pixels);

    CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

    TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);

}

void RHIVulkan::CreateTextureImageView()
{
    m_textureImageView = CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void RHIVulkan::CreateTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(m_vkDevice, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void RHIVulkan::CreateSampler(
    VkSampler& outSampler,
    const VkFilter filter,
    const VkSamplerAddressMode addressModeU,
    const VkSamplerAddressMode addressModeV,
    const VkSamplerAddressMode addressModeW,
    const VkBorderColor borderColor,
    const uint32_t miplevels)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    samplerInfo.addressModeU = addressModeU;
    samplerInfo.addressModeV = addressModeV;
    samplerInfo.addressModeW = addressModeW;
    // 可在此处关闭各项异性，有些硬件可能不支持
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = borderColor;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(miplevels);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(m_vkDevice, &samplerInfo, nullptr, &outSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to Create texture sampler!");
    }
}

void RHIVulkan::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommandBuffer(commandBuffer);
}

void RHIVulkan::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndSingleTimeCommandBuffer(commandBuffer);
}

void RHIVulkan::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


    //begin recod command
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin record command buffer !");
    }

    //shadow map
    {
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_shadowmapPass.RenderPass;
        renderPassInfo.framebuffer = m_shadowmapPass.FrameBuffer;
        renderPassInfo.renderArea.extent.width = m_shadowmapPass.Width;
        renderPassInfo.renderArea.extent.height = m_shadowmapPass.Height;
        std::array<VkClearValue, 1> clearValues{};
        clearValues[0].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = clearValues.data();

        //begin
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //viewport
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_shadowmapPass.Width;
        viewport.height = (float)m_shadowmapPass.Height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        //scissor
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = (float)m_shadowmapPass.Width;
        scissor.extent.width = (float)m_shadowmapPass.Height;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        //avoid artifacts
        float depthBiasConstant = 1.25f;
        float depthBiasSlope = 7.5; // change from 1.75f to fix PCF artifact
        vkCmdSetDepthBias(
            commandBuffer,
            depthBiasConstant,
            0.0f,
            depthBiasSlope);

        for (auto item : m_baseScenePass.RenderObjects)
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowmapPass.Pipeline);
            VkBuffer objectVertexBuffers[] = { item.meshData.VertexBuffer };
            VkDeviceSize objectOffsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, objectVertexBuffers, objectOffsets);
            vkCmdBindIndexBuffer(commandBuffer, item.meshData.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_shadowmapPass.PipelineLayout, 0, 1, &m_shadowmapPass.DescriptorSets[m_currentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(item.meshData.Indices.size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);
    }


    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_vkRenderPass;
        renderPassInfo.framebuffer = m_vkSwapChainFrameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_vkSwapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.3f, 0.5f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_vkSwapChainExtent.width;
        viewport.height = (float)m_vkSwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_vkSwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


        for (auto item : m_baseScenePass.RenderObjects)
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_baseScenePass.Pipelines[0]);

            VkBuffer vertexBuffers[] = { item.meshData.VertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, item.meshData.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_baseScenePass.PipelineLayout, 0, 1, &m_baseScenePass.RenderObjects[0].mat.DescriptorSets[m_currentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(item.meshData.Indices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(commandBuffer);

        }
    }

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

VkCommandBuffer RHIVulkan::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vkCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;//执行一次立即刷新

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void RHIVulkan::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &commandBuffer);
}

void RHIVulkan::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    EndSingleTimeCommandBuffer(commandBuffer);
}

void RHIVulkan::CreateVertexBuffer(VkBuffer& outBuffer, VkDeviceMemory& outMemory, const std::vector<Vertex>& inVertex)
{
    VkDeviceSize bufferSize = sizeof(inVertex[0]) * inVertex.size();

    // 为什么需要stagingBuffer，因为直接创建VertexBuffer，CPU端可以直接通过vertexBufferMemory范围GPU使用的内存，这样太危险了，
    // 所以我们先创建一个临时的Buffer写入数据，然后将这个Buffer拷贝给最终的VertexBuffer，
    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT标签，使得最终的VertexBuffer位于硬件本地内存中，比如显卡的显存。
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, inVertex.data(), size_t(bufferSize));
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outBuffer, outMemory);
    CopyBuffer(stagingBuffer, outBuffer, bufferSize);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);
}

void RHIVulkan::CreateIndexBuffer(VkBuffer& outBuffer, VkDeviceMemory& outMemory, const std::vector<uint32_t>& inIndices)
{
    VkDeviceSize bufferSize = sizeof(inIndices[0]) * inIndices.size();

    // 为什么需要stagingBuffer，因为直接创建VertexBuffer，CPU端可以直接通过vertexBufferMemory范围GPU使用的内存，这样太危险了，
    // 所以我们先创建一个临时的Buffer写入数据，然后将这个Buffer拷贝给最终的VertexBuffer，
    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT标签，使得最终的VertexBuffer位于硬件本地内存中，比如显卡的显存。
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, inIndices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_vkDevice, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outBuffer, outMemory);
    CopyBuffer(stagingBuffer, outBuffer, bufferSize);

    vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_vkDevice, stagingBufferMemory, nullptr);
}

uint32_t RHIVulkan::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    // 自动寻找适合的内存类型
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void RHIVulkan::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_vkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw  std::runtime_error("failed to create buffer!!!!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_vkDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
    if (vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory");
    }
    vkBindBufferMemory(m_vkDevice, buffer, bufferMemory, 0);
}

void RHIVulkan::CreateCommandBuffers()
{
    m_vkCommandBuffer.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_vkCommandBuffer.size();

    if (vkAllocateCommandBuffers(m_vkDevice, &allocInfo, m_vkCommandBuffer.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    /*
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_vkCommandBuffer[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_vkRenderPass;
        renderPassInfo.framebuffer = m_vkSwapChainFrameBuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_vkSwapChainExtent;

        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_vkCommandBuffer[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_vkCommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkGraphicsPipeline);

        vkCmdDraw(m_vkCommandBuffer[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(m_vkCommandBuffer[i]);

        if (vkEndCommandBuffer(m_vkCommandBuffer[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
     */
}

void RHIVulkan::CreateSyncObjects()
{

    m_vkImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vkInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_vkImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_vkDevice, &semaphoreInfo, nullptr, &m_vkRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_vkDevice, &fenceInfo, nullptr, &m_vkInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

}

void RHIVulkan::DrawFrame() {
    vkWaitForFences(m_vkDevice, 1, &m_vkInFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result =vkAcquireNextImageKHR(
        m_vkDevice, m_vkSwapChain, UINT64_MAX, m_vkImageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex
    );
    
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        ReCreateSwapChain();
        return;
    }

    UpdateUniformBuffer(m_currentFrame);
    vkResetFences(m_vkDevice, 1, &m_vkInFlightFences[m_currentFrame]);
    
    vkResetCommandBuffer(m_vkCommandBuffer[m_currentFrame], 0);
    RecordCommandBuffer(m_vkCommandBuffer[m_currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_vkImageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_vkCommandBuffer[m_currentFrame];

    VkSemaphore signalSemaphores[] = { m_vkRenderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_vkInFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_vkSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RHIVulkan::Cleanup() {

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT;i++)
    {
        // 先创建的东西后销毁
        vkDestroySemaphore(m_vkDevice, m_vkImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_vkDevice, m_vkImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_vkDevice, m_vkInFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, nullptr);

    for (auto framebuffer : m_vkSwapChainFrameBuffers) {
        vkDestroyFramebuffer(m_vkDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(m_vkDevice, m_vkGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_vkDevice, m_vkPipeLineLayout, nullptr);
    vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, nullptr);

    vkDestroyBuffer(m_vkDevice, m_vertBuffers, nullptr);
    vkFreeMemory(m_vkDevice, m_vertBufferMemory, nullptr);
    vkDestroyBuffer(m_vkDevice, m_indexBuffers, nullptr);
    vkFreeMemory(m_vkDevice, m_indexBufferMemory, nullptr);
    
    vkDestroyImageView(m_vkDevice, m_depthImageView, nullptr);
    vkDestroyImage(m_vkDevice, m_depthImage, nullptr);
    vkFreeMemory(m_vkDevice, m_depthImageMemory, nullptr);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_vkDevice, m_vkUniformBuffers[i], nullptr);
        vkFreeMemory(m_vkDevice, m_vkUniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);

    for (auto imageView : m_vkSwapChainImageViews) {
        vkDestroyImageView(m_vkDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, nullptr);
    vkDestroyDevice(m_vkDevice, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vkInstance, m_surfaceWindow, nullptr);
    vkDestroyInstance(m_vkInstance, nullptr);
    glfwDestroyWindow(m_glfwWindow);
    glfwTerminate();
}
