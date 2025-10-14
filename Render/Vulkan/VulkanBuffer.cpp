#include "VulkanTool.h"
#include "VulkanBuffer.h"

VkResult Render::Vulkan::Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    return vkMapMemory(device, memory, offset, size, 0, &mapped);
}

void Render::Vulkan::Buffer::UnMap()
{
    if(mapped)
    {
        vkUnmapMemory(device, memory);
        mapped = nullptr;
    }
}

VkResult Render::Vulkan::Buffer::Bind(VkDeviceSize offset)
{
    return vkBindBufferMemory(device, buffer, memory, offset);
}

void Render::Vulkan::Buffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    descriptor.offset = offset;
    descriptor.range = size;
    descriptor.buffer = buffer;
}

void Render::Vulkan::Buffer::CopyTo(void* data, VkDeviceSize size)
{
    if(!mapped) return;
    memcpy(mapped, data, size);
}

VkResult Render::Vulkan::Buffer::Flush(VkDeviceSize size, VkDeviceSize offset )
{
    VkMappedMemoryRange mappedRange{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = memory,
        .offset = offset,
        .size = size
    };

    return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
}

VkResult Render::Vulkan::Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = memory,
            .offset = offset,
            .size = size
    };
    return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
}

void Render::Vulkan::Buffer::Destroy()
{
    if(buffer)
    {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }

    if(memory)
    {
        vkFreeMemory(device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}










































































