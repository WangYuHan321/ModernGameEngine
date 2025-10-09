#if defined __ANDROID__
#endif

#include "VulkanAndroid.h"
#include "VulkanBuffer.h"

VkResult vks::Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    return vkMapMemory(device, memory, offset, size, 0, &mapped);
}

void vks::Buffer::UnMap()
{
    if(mapped)
    {
        vkUnmapMemory(device, memory);
        mapped = nullptr;
    }
}

VkResult vks::Buffer::Bind(VkDeviceSize offset)
{
    return vkBindBufferMemory(device, buffer, memory, offset);
}

void vks::Buffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    descriptor.offset = offset;
    descriptor.range = size;
    descriptor.buffer = buffer;
}

void vks::Buffer::CopyTo(void* data, VkDeviceSize size)
{
    if(!mapped) return;
    memcpy(mapped, data, size);
}

VkResult vks::Buffer::Flush(VkDeviceSize size, VkDeviceSize offset )
{
    VkMappedMemoryRange mappedRange{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = memory,
        .offset = offset,
        .size = size
    };

    return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
}

VkResult vks::Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = memory,
            .offset = offset,
            .size = size
    };
    return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
}

void vks::Buffer::Destroy()
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










































































