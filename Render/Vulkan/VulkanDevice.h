#pragma  once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "VulkanBuffer.h"

namespace Render
{
    namespace Vulkan
    {
        class VulkanDevice
        {
        public:
        struct QueueFamilyIndices
        {
            uint32_t  graphics;
            uint32_t compute;
            uint32_t transfer;
        };

          VkPhysicalDevice physicalDevice { VK_NULL_HANDLE};

          VkDevice logicalDevice{ VK_NULL_HANDLE };

          VkPhysicalDeviceProperties properties{};

          VkPhysicalDeviceFeatures features{};

          VkPhysicalDeviceFeatures enableFeatures {};

          VkPhysicalDeviceMemoryProperties memoryProperties{};

          std::vector<VkQueueFamilyProperties> queueFamilyProperties{};

          std::vector<std::string> supportedExtensions{};

          VkCommandPool commandPool { VK_NULL_HANDLE };

          QueueFamilyIndices queueFamilyIndices;

          operator VkDevice() const
          {
              return logicalDevice;
          }

          explicit  VulkanDevice(VkPhysicalDevice _physicalDevice1);
          ~VulkanDevice();

          uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr) const;
          uint32_t GetQueueFamilyIndex(VkQueueFlags flag) const;

          VkResult CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions,
                                       void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
          VkResult CreateBuffer(VkBufferUsageFlags useFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory* memory, void*data = nullptr);
          VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Render::Vulkan::Buffer *buffer, VkDeviceSize size, void *data = nullptr);

          void CopyBuffer(Render::Vulkan::Buffer *src, Render::Vulkan::Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion = nullptr);

          VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
          VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
          VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
          void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
          void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
          bool ExtensionSupported(std::string extension);
          VkFormat GetSupportedDepthFormat(bool checkSamplingSupport);

        };
    }
}