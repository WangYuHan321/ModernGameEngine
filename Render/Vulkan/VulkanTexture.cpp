#include "VulkanTexture.h"

void Render::Vulkan::VulkanTexture::UpdateDescriptor()
{
	descirptor.sampler = sampler;
	descirptor.imageView = imageView;
	descirptor.imageLayout = imageLayout;
}

void Render::Vulkan::VulkanTexture::Destory()
{
	vkDestroyImageView(device->logicalDevice, imageView, nullptr);
	vkDestroyImage(device->logicalDevice, image, nullptr);
	if (sampler)
	{
		vkDestroySampler(device->logicalDevice, sampler, nullptr);
	}
	vkFreeMemory(device->logicalDevice, deviceMemory, nullptr);
}

void Render::Vulkan::VulkanTexture2D::LoadFromFile()
{
}

void Render::Vulkan::VulkanTexture2D::FromBuffer(void* buffer,
	VkDeviceSize       bufferSize,
	VkFormat           format,
	uint32_t           texWidth,
	uint32_t           texHeight,
	Render::Vulkan::VulkanDevice* device,
	VkQueue            copyQueue,
	VkFilter           filter = VK_FILTER_LINEAR,
	VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
	VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
{
}

void Render::Vulkan::VulkanTexture2DArray::LoadFromFile()
{
}

void Render::Vulkan::VulkanTextureCube::LoadFromFile()
{
}
