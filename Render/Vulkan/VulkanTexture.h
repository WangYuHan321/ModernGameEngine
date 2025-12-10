#pragma once
#include "VulkanBuffer.h"
#include "VulkanDevice.h"

namespace Render
{
    namespace Vulkan
    {
        class VulkanTexture
        {
        public:
            Vulkan::VulkanDevice* device;
            VkImage image;
            VkImageLayout imageLayout;
            VkDeviceMemory deviceMemory;
            VkImageView imageView;
            uint32_t width, height;
            uint32_t mipLevels = 1;
            uint32_t layerCount;
            VkDescriptorImageInfo descirptor;
            VkSampler sampler;

            void UpdateDescriptor();
            void Destory();
        };

        class VulkanTexture2D : public VulkanTexture
        {
        public:
            void LoadFromFile(
                std::string        filename,
                VkFormat           format,
                Vulkan::VulkanDevice* device,
                VkQueue            copyQueue,
                VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
                VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            void FromBuffer(
                void* buffer,
                VkDeviceSize       bufferSize,
                VkFormat           format,
                uint32_t           texWidth,
                uint32_t           texHeight,
                Vulkan::VulkanDevice* device,
                VkQueue            copyQueue,
                VkFilter           filter = VK_FILTER_LINEAR,
                VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
                VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        };

        class VulkanTexture2DArray : public VulkanTexture
        {
        public:
            void LoadFromFile(
                std::vector<std::string> fileNameVec,
                VkFormat           format,
                Vulkan::VulkanDevice* device,
                VkQueue            copyQueue,
                VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
                VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
        };

        class VulkanTextureCubeMap : public VulkanTexture
        {
        public:
            void LoadFromFile(
                std::vector<std::string> filename,
                VkFormat           format,
                Vulkan::VulkanDevice* device,
                VkQueue            copyQueue,
                VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
                VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        };
    }
}