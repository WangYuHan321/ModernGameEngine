﻿#include "VulkanTexture.h"
#include "VulkanTool.h"
#include <stdexcept>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanInitializers.hpp"

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

void Render::Vulkan::VulkanTexture2D::LoadFromFile(
	std::string        filename,
	VkFormat           format,
	Vulkan::VulkanDevice* device,
	VkQueue            copyQueue,
	VkImageUsageFlags  imageUsageFlags,
	VkImageLayout      imageLayout)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	//get device property
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProps);

	VkCommandBuffer copyCmd = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = Render::Vulkan::Initializer::BufferCreateInfo(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		imageSize);
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //����ģʽ

	VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));
	
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);
	VkMemoryAllocateInfo memAlloc = Vulkan::Initializer::MemoryAllocInfo();
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = device->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAlloc, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

	void* data;
	vkMapMemory(device->logicalDevice, stagingMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device->logicalDevice, stagingMemory);

	stbi_image_free(pixels);

	VkImageCreateInfo imageCreateInfo = Vulkan::Initializer::ImageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = imageUsageFlags;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &image));
	vkGetImageMemoryRequirements(device->logicalDevice, image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = device->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAlloc, nullptr, &deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));

	VkImageSubresourceRange subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = mipLevels, .layerCount = 1, };

	//transition image layout
	Vulkan::Tool::SetImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);
	
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	this->imageLayout = imageLayout;
	Vulkan::Tool::SetImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageLayout,
		subresourceRange);

	Vulkan::Tool::GenerateMipMap(device, copyQueue, image, format, texWidth, texHeight, mipLevels );

	vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);

	// Create a default sampler
	VkSamplerCreateInfo samplerCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = device->enableFeatures.samplerAnisotropy,
		.maxAnisotropy = device->enableFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f,
		.compareOp = VK_COMPARE_OP_NEVER,
		.minLod = 0.0f,
		.maxLod = (float)mipLevels,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	};
	VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerCreateInfo, nullptr, &sampler));

	VkImageViewCreateInfo viewCreateInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = mipLevels, .baseArrayLayer = 0, .layerCount = 1 },
	};
	VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &imageView));

	// Update descriptor image info member that can be used for setting up descriptor sets
	UpdateDescriptor();
}

void Render::Vulkan::VulkanTexture2D::FromBuffer(void* buffer,
	VkDeviceSize       bufferSize,
	VkFormat           format,
	uint32_t           texWidth,
	uint32_t           texHeight,
	Render::Vulkan::VulkanDevice* device,
	VkQueue            copyQueue,
	VkFilter           filter,
	VkImageUsageFlags  imageUsageFlags,
	VkImageLayout      imageLayout)
{
}

void Render::Vulkan::VulkanTexture2DArray::LoadFromFile()
{
}

void Render::Vulkan::VulkanTextureCube::LoadFromFile()
{
}
