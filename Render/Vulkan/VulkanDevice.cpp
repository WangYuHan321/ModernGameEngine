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

}

uint32_t Render::Vulkan::VulkanDevice::GetQueueFamilyIndex(VkQueueFlags flag) const
{

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

}

bool Render::Vulkan::VulkanDevice::ExtensionSupported(std::string extension)
{

}

VkFormat Render::Vulkan::VulkanDevice::GetSupportedDepthFormat(bool checkSamplingSupport)
{

}