#include "VulkanDevice.h"
#include "VulkanTool.h"
#include <unordered_map>
#include <assert.h>

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT))
#define VK_ENABLE_BETA_EXTENSIONS
#endif

Render::Vulkan::VulkanDevice::VulkanDevice(VkPhysicalDevice _physicalDevice1)
{
    assert(physicalDevice);

    this->physicalDevice = _physicalDevice1;

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    uint32_t  queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueFamilyCount, nullptr);
    assert(queueFamilyCount);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    uint32_t extCount = 0;
    if(extCount > 0)
    {
        std::vector<VkExtensionProperties> extensions(extCount);
        if(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
        {
            for(auto& ext : extensions)
            {
                supportedExtensions.push_back(ext.extensionName);
            }
        }
    }
}

Render::Vulkan::VulkanDevice::~VulkanDevice()
{
    if(commandPool)
    {
        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    }
    if(logicalDevice)
    {
        vkDestroyDevice(logicalDevice, nullptr);
    }
}

uint32_t Render::Vulkan::VulkanDevice::GetMemoryType( uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound ) const
{
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = false;
        return 0;
    }
    else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}

uint32_t Render::Vulkan::VulkanDevice::GetQueueFamilyIndex(VkQueueFlags flag) const
{
    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if ((flag & VK_QUEUE_COMPUTE_BIT) == flag)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
            {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if ((flag & VK_QUEUE_TRANSFER_BIT) == flag)
    {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
            {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
    {
        if ((queueFamilyProperties[i].queueFlags & flag) == flag)
        {
            return i;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

VkResult Render::Vulkan::VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions,
                             void* pNextChain, bool useSwapChain, VkQueueFlags requestedQueueType)
{

}

VkResult Render::Vulkan::VulkanDevice::CreateBuffer(VkBufferUsageFlags useFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory* memory, void*data)
{

}

VkResult Render::Vulkan::VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Render::Vulkan::Buffer *buffer, VkDeviceSize size, void *data)
{

}

void Render::Vulkan::VulkanDevice::CopyBuffer(Render::Vulkan::Buffer *src, Render::Vulkan::Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion)
{

}

VkCommandPool Render::Vulkan::VulkanDevice::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{

}

VkCommandBuffer Render::Vulkan::VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{

}

VkCommandBuffer Render::Vulkan::VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
{

}

void Render::Vulkan::VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
{

}

void Render::Vulkan::VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
{
    return FlushCommandBuffer(commandBuffer, queue, commandPool, free);
}

bool Render::Vulkan::VulkanDevice::ExtensionSupported(std::string extension)
{
    return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

VkFormat Render::Vulkan::VulkanDevice::GetSupportedDepthFormat(bool checkSamplingSupport)
{
    // All depth formats may be optional, so we need to find a suitable depth format to use
    std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
    for (auto& format : depthFormats)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            if (checkSamplingSupport) {
                if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
                    continue;
                }
            }
            return format;
        }
    }
    throw std::runtime_error("Could not find a matching depth format");
}