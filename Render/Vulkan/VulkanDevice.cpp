#include "VulkanDevice.h"
#include "VulkanTool.h"
#include "VulkanInitializers.hpp"
#include <unordered_map>
#include <assert.h>
#include <stdexcept>
#include <iostream>

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT))
#define VK_ENABLE_BETA_EXTENSIONS
#endif

Render::Vulkan::VulkanDevice::VulkanDevice(VkPhysicalDevice _physicalDevice1)
{
    assert(_physicalDevice1);

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
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	const float defaultQueuePriority(0.0f);


    if (requestedQueueType & VK_QUEUE_GRAPHICS_BIT)
    {
        queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    }
    else
    {
        queueFamilyIndices.graphics = 0;
    }

    if (requestedQueueType & VK_QUEUE_COMPUTE_BIT)
    {
        queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    }
    else
    {
        queueFamilyIndices.compute = 0;
    }

    if (requestedQueueType & VK_QUEUE_TRANSFER_BIT)
    {
        queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    }
    else
    {
        queueFamilyIndices.transfer = 0;
    }

    std::vector<const char*> deviceExtensions(enabledExtensions);
    
    if (useSwapChain)
    {
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

    // If a pNext(Chain) has been passed, we need to add it to the device creation info
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    if (pNextChain)
    {
        physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        physicalDeviceFeatures2.features = enableFeatures;
        physicalDeviceFeatures2.pNext = pNextChain;
        deviceCreateInfo.pEnabledFeatures = nullptr;
        deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    }

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)) && defined(VK_KHR_portability_subset)
    // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_subset is defined and supported by the device, enable the extension
    if (extensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
    {
        deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif

    if (deviceExtensions.size() > 0)
    {
        for (const char* enabledExtension : deviceExtensions)
        {
            if (!ExtensionSupported(enabledExtension)) {
                std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
            }
        }

        deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    }

    this->enableFeatures = enabledFeatures;

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS)
        return result;

    // Create a default command pool for graphics command buffers
    commandPool = CreateCommandPool(queueFamilyIndices.graphics);
    return result;
}

VkResult Render::Vulkan::VulkanDevice::CreateBuffer(VkBufferUsageFlags useFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory* memory, void*data)
{
    VkBufferCreateInfo bufferCreateInfo = Render::Vulkan::Initializer::BufferCreateInfo(useFlags, size);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer));


    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = Vulkan::Initializer::MemoryAllocInfo();
    vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (useFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory));

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        void* mapped;
        VK_CHECK_RESULT(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            VkMappedMemoryRange mappedRange = Vulkan::Initializer::MappedMemoryRange();
            mappedRange.memory = *memory;
            mappedRange.offset = 0;
            mappedRange.size = size;
            vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
        }
        vkUnmapMemory(logicalDevice, *memory);
    }

    // Attach the memory to the buffer object
    VK_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

    return VK_SUCCESS;

}

VkResult Render::Vulkan::VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Render::Vulkan::Buffer *buffer, VkDeviceSize size, void *data)
{
    buffer->device = logicalDevice;

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = Vulkan::Initializer::BufferCreateInfo(usageFlags, size);
    VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer->buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = Vulkan::Initializer::MemoryAllocInfo();
    vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &buffer->memory));

    buffer->alignment = memReqs.alignment;
    buffer->size = size;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        VK_CHECK_RESULT(buffer->Map());
        memcpy(buffer->mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            buffer->Flush();

        buffer->UnMap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->SetupDescriptor();

    // Attach the memory to the buffer object
    return buffer->Bind();
}

void Render::Vulkan::VulkanDevice::CopyBuffer(Render::Vulkan::Buffer *src, Render::Vulkan::Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion)
{
    assert(dst->size <= src->size);
    assert(src->buffer);
    VkCommandBuffer copyCmd = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy bufferCopy{};
    if (copyRegion == nullptr)
    {
        bufferCopy.size = src->size;
    }
    else
    {
        bufferCopy = *copyRegion;
    }

    vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);

    //??????CPU??????
    FlushCommandBuffer(copyCmd, queue);
}

VkCommandPool Render::Vulkan::VulkanDevice::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
    VkCommandPoolCreateInfo cmdPoolInfo{};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
    cmdPoolInfo.flags = createFlags;
    VkCommandPool cmdPool;
    VK_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
    return cmdPool;
}

VkCommandBuffer Render::Vulkan::VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = Vulkan::Initializer::CommandBufferAllocateInfo(pool, level, 1);
    VkCommandBuffer cmdBuffer;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
    // If requested, also start recording for the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = Vulkan::Initializer::CommandBufferBeginInfo();
        VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
    }
    return cmdBuffer;
}

VkCommandBuffer Render::Vulkan::VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
{
    return CreateCommandBuffer(level, commandPool, begin);
}

void Render::Vulkan::VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
{
    //??CPU
    if (commandBuffer == VK_NULL_HANDLE)
        return;

    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = Vulkan::Initializer::SubmitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkFenceCreateInfo fenceInfo = Vulkan::Initializer::FenceCreateInfo(VK_FLAGS_NONE);
    VkFence fence;// VkFence fence[] = {  x,x,x };
    VK_CHECK_RESULT(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fence));

    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

    VK_CHECK_RESULT(vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
    vkDestroyFence(logicalDevice, fence, nullptr);
    if (free)
    {
        vkFreeCommandBuffers(logicalDevice, pool, 1, &commandBuffer);
    }
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