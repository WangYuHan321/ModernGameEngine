#include "VulkanTool.h"
#include "VulkanInitializers.hpp"

void Render::Vulkan::Tool::SetImageLayout(
	VkCommandBuffer commandBuffer,
	VkImage         image,
	VkImageLayout   oldImageLayout,
	VkImageLayout   newImageLayout,
	VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	/*oldImageLayout��ͼƬ��ǰ�Ĳ��֡�
	newImageLayout������Ҫ�л����Ĳ��֡�
	srcAccessMask�����л�ǰ��Ҫ��� / �ɼ��ķ������ͣ�ȷ���ڽ����²���ǰ��Щ�����Ѿ���ɡ��Ժ������ʿɼ���
	dstAccessMask���л����²��ֺ�Ŀ�����������Ҫ���ڴ� / ִ�����ϡ�*/

	VkImageMemoryBarrier imageMemoryBarrier = Render::Vulkan::Initializer::ImageMemoryBarrier();
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	switch (oldImageLayout)
	{	
	case VK_IMAGE_LAYOUT_UNDEFINED://ͼƬ��ǰ�޶��岼�֣������ڳ�ʼ״̬�����������ݡ�
		srcStageMask = 0;//û���ѷ����ķ�����Ҫ�ȴ���
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED://ͼƬ�ѱ�����/CPU Ԥ��ʼ��д���������ͼƬ�ϳ�������
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;//��֤������д����ɺ��豸�ٷ���ͼƬ��
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: //ͼƬ����Ϊ��ɫ����ʹ�ã���ȾĿ�꣩��
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//ȷ���ڽ����²���ǰ����ɫ����д���Ѿ���ɡ�
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL://ͼƬ����Ϊ���/ģ�壨���/ģ�建������ʹ�á�
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;//ȷ���ڽ����²���ǰ�������ȡ/��ز�����ɡ�
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL://ͼƬ����Ϊ����Դʹ�ã������ݣ���
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;//ȷ���ڽ����²���ǰ�������ȡ/��ز�����ɡ�
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL://ͼƬ����Ϊ����Ŀ��ʹ�ã�д���ݣ���
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;//ȷ���ڽ����²���ǰ������д����ɡ�
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL://ͼƬ������ɫ��ֻ�����ʣ������������
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;//ȷ��֮ǰ��д�����ɫ���ɼ���
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL://ͼƬ����Ϊ����Ŀ��ʹ�ã�д���ݣ���
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;//Ŀ����д�뵽ͼƬ��ͨ��������С���
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;//Ŀ���Ǵ�ͼƬ��ȡ��ͨ��������С���
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//Ŀ������Ϊ��ɫ����д�롣
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL://Ŀ������Ϊ���/ģ�帽��д�룻ע���� OR ���⸲��ԭ��ֵ��
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (imageMemoryBarrier.srcAccessMask == 0)//ͼƬ������ɫ��ֻ�����ʣ������������
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;//֮ǰû����ʽָ�����κη��ʣ������ UNDEFINED ת��ֻ�������ͼ������������д�����д����Ҫͬ����ȷ����Щд�����ɫ����ȡ�ɼ���
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

VkCommandBuffer BeginSingleTimeCommands(Render::Vulkan::VulkanDevice* device, VkCommandPool cmdPool)
{
	// ����Ⱦһ����ʹ��CommandBuffer��������
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = cmdPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void EndSingleTimeCommands(Render::Vulkan::VulkanDevice* device, VkCommandPool cmdPool, VkCommandBuffer commandBuffer,VkQueue queue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device->logicalDevice, cmdPool, 1, &commandBuffer);
}

void Render::Vulkan::Tool::GenerateMipMap(Render::Vulkan::VulkanDevice* device, 
	VkQueue queue, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	// ���ͼ���ʽ�Ƿ�֧�� linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, device->commandPool);

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

	EndSingleTimeCommands(device, device->commandPool,commandBuffer, queue);
}

