#pragma once

#include <algorithm>
#include <iterator>
#include <vector>
#include <array>
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"
#include "VulkanTool.h"
#include "VulkanInitializers.hpp"

namespace Render
{
	namespace Vulkan
	{
		struct VulkanFramebufferAttachment
		{
			VkImage image;
			VkDeviceMemory memory;
			VkImageView imageView;
			VkFormat format;
			VkImageSubresourceRange subResourceRange;
			VkAttachmentDescription description;

			bool HasDepth()
			{
				std::vector<VkFormat> formats =
				{
					VK_FORMAT_D16_UNORM,
					VK_FORMAT_X8_D24_UNORM_PACK32,
					VK_FORMAT_D32_SFLOAT,
					VK_FORMAT_D16_UNORM_S8_UINT,
					VK_FORMAT_D24_UNORM_S8_UINT,
					VK_FORMAT_D32_SFLOAT_S8_UINT,
				};
				return std::find(formats.begin(), formats.end(), format) != std::end(formats);
			}

			bool HasStencil()
			{
				std::vector<VkFormat> formats =
				{
					VK_FORMAT_S8_UINT,
					VK_FORMAT_D16_UNORM_S8_UINT,
					VK_FORMAT_D24_UNORM_S8_UINT,
					VK_FORMAT_D32_SFLOAT_S8_UINT,
				};
				return std::find(formats.begin(), formats.end(), format) != std::end(formats);
			}
			
			bool IsDepthStencil()
			{
				return(HasDepth() || HasStencil());
			}
		};

		struct AttachmentCreateInfo
		{
			uint32_t width, height;
			uint32_t layerCount;
			VkFormat format;
			VkImageUsageFlags usage;
			VkSampleCountFlagBits imageSamplerCount = VK_SAMPLE_COUNT_1_BIT;
		};

		class VulkanFrameBuffer
		{
		private:
			Render::Vulkan::VulkanDevice* vulkanDevice;
		public:
			uint32_t width, height;
			VkFramebuffer frameBuffer;
			VkRenderPass renderPass;
			VkSampler sampler;
			std::vector<Render::Vulkan::VulkanFramebufferAttachment> attachments;

			VulkanFrameBuffer(Render::Vulkan::VulkanDevice* vulkanDevice)
			{
				assert(vulkanDevice);
				this->vulkanDevice = vulkanDevice;
			}

			~VulkanFrameBuffer()
			{
				assert(vulkanDevice);

				for (auto attachment : attachments)
				{
					vkDestroyImage(vulkanDevice->logicalDevice, attachment.image, nullptr);
					vkDestroyImageView(vulkanDevice->logicalDevice, attachment.imageView, nullptr);
					vkFreeMemory(vulkanDevice->logicalDevice, attachment.memory, nullptr);
				}
				vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
				vkDestroyRenderPass(vulkanDevice->logicalDevice, renderPass, nullptr);
				vkDestroyFramebuffer(vulkanDevice->logicalDevice, frameBuffer, nullptr);
			}

			uint32_t AddAttachment(Render::Vulkan::AttachmentCreateInfo createInfo)
			{
				Render::Vulkan::VulkanFramebufferAttachment attachment;
				attachment.format = createInfo.format;

				VkImageAspectFlags aspecMask = PEN_FLAG_NONE;

				//Color Attachment
				if (createInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
				{
					aspecMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}

				//Depth Attachment
				if (createInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					if (attachment.HasDepth())
					{
						aspecMask = VK_IMAGE_ASPECT_DEPTH_BIT;
					}
					if (attachment.HasStencil())
					{
						aspecMask = aspecMask | VK_IMAGE_ASPECT_STENCIL_BIT;
					}
				}

				assert(aspecMask > 0);

				VkImageCreateInfo image = Render::Vulkan::Initializer::ImageCreateInfo();
				image.imageType = VK_IMAGE_TYPE_2D;
				image.format = createInfo.format;
				image.extent.width = createInfo.width;
				image.extent.height = createInfo.height;
				image.extent.depth = 1;
				image.mipLevels = 1;
				image.arrayLayers = createInfo.layerCount;
				image.samples = createInfo.imageSamplerCount;
				image.tiling = VK_IMAGE_TILING_OPTIMAL;
				image.usage = createInfo.usage;

				VkMemoryAllocateInfo memAlloc = Render::Vulkan::Initializer::MemoryAllocInfo();
				VkMemoryRequirements memReqs;

				VK_CHECK_RESULT(vkCreateImage(vulkanDevice->logicalDevice, &image, nullptr, &attachment.image));
				vkGetImageMemoryRequirements(vulkanDevice->logicalDevice, attachment.image, &memReqs);
				memAlloc.allocationSize = memReqs.size;
				memAlloc.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				VK_CHECK_RESULT(vkAllocateMemory(vulkanDevice->logicalDevice, &memAlloc, nullptr, &attachment.memory));
				VK_CHECK_RESULT(vkBindImageMemory(vulkanDevice->logicalDevice, attachment.image, attachment.memory, 0));

				attachment.subResourceRange = {};
				attachment.subResourceRange.aspectMask = aspecMask;
				attachment.subResourceRange.levelCount = 1;
				attachment.subResourceRange.layerCount = createInfo.layerCount;

				VkImageViewCreateInfo imageView = Render::Vulkan::Initializer::ImageViewCreateInfo();
				imageView.viewType = (createInfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				imageView.format = createInfo.format;
				imageView.subresourceRange = attachment.subResourceRange;
				imageView.subresourceRange.aspectMask = (attachment.HasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspecMask;
				imageView.image = attachment.image;
				VK_CHECK_RESULT(vkCreateImageView(vulkanDevice->logicalDevice, &imageView, nullptr, &attachment.imageView));

				//(createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
				//如果图像被标记为要被采样（future shader sampling），就把 attachment 的 storeOp 设为 STORE，保证渲染结果被保留以便后续读取。
				//如果图像不会被采样，就可以用 DONT_CARE，让驱动有机会在性能或内存上做优化（例如不把瓦片写回主内存）。
				// 
				// Fill attachment description
				attachment.description = {};
				attachment.description.samples = createInfo.imageSamplerCount;
				attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment.description.storeOp = (createInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.description.format = createInfo.format;
				attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				// Final layout
				// If not, final layout depends on attachment type
				if (attachment.HasDepth() || attachment.HasStencil())
				{
					attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				}
				else
				{
					attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}

				attachments.push_back(attachment);

				return static_cast<uint32_t>(attachments.size() - 1);
			}

			VkResult CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode)
			{
				VkSamplerCreateInfo samplerInfo = Render::Vulkan::Initializer::SamplerCreateInfo();
				samplerInfo.magFilter = magFilter;
				samplerInfo.minFilter = minFilter;
				samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerInfo.addressModeU = addressMode;
				samplerInfo.addressModeV = addressMode;
				samplerInfo.addressModeW = addressMode;
				samplerInfo.mipLodBias = 0.0f;
				samplerInfo.maxAnisotropy = 1.0f;
				samplerInfo.minLod = 0.0f;
				samplerInfo.maxLod = 1.0f;
				samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				return vkCreateSampler(vulkanDevice->logicalDevice, &samplerInfo, nullptr, &sampler);
			}

			VkResult CreateRenderPass()
			{
				std::vector<VkAttachmentDescription> attachmentDescriptions;
				for (auto& attachment : attachments)
					attachmentDescriptions.push_back(attachment.description);

				std::vector<VkAttachmentReference> colorReference;
				VkAttachmentReference depthReference;
				bool hasDepth = false;
				bool hasColor = false;

				uint32_t attachmentIndex = 0;

				for (auto& attachment : attachments)
				{
					if (attachment.IsDepthStencil())
					{
						assert(!hasDepth);
						depthReference.attachment = attachmentIndex;
						depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						hasDepth = true;
					}
					else
					{
						colorReference.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
						hasColor = true;
					}
					attachmentIndex++;
				};

				VkSubpassDescription subpass = {};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				if (hasColor)
				{
					subpass.pColorAttachments = colorReference.data();
					subpass.colorAttachmentCount = static_cast<uint32_t>(colorReference.size());
				}
				if (hasDepth)
				{
					subpass.pDepthStencilAttachment = &depthReference;
				};

				std::array<VkSubpassDependency, 2> dependencies;

				dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[0].dstSubpass = 0;
				dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				dependencies[1].srcSubpass = 0;
				dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				VkRenderPassCreateInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.pAttachments = attachmentDescriptions.data();
				renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;
				renderPassInfo.dependencyCount = 2;
				renderPassInfo.pDependencies = dependencies.data();
				VK_CHECK_RESULT(vkCreateRenderPass(vulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass));

				std::vector<VkImageView> attachmentViews;
				for (auto attachment : attachments)
				{
					attachmentViews.push_back(attachment.imageView);
				}
					

				uint32_t maxLayers = 0;
				for (auto attachment : attachments)
				{
					if (attachment.subResourceRange.layerCount > maxLayers)
					{
						maxLayers = attachment.subResourceRange.layerCount;
					}
				}

				VkFramebufferCreateInfo frameBufferInfo{};
				frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				frameBufferInfo.renderPass = renderPass;
				frameBufferInfo.pAttachments = attachmentViews.data();
				frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
				frameBufferInfo.width = width;
				frameBufferInfo.height = height;
				frameBufferInfo.layers = maxLayers;
				VK_CHECK_RESULT(vkCreateFramebuffer(vulkanDevice->logicalDevice, &frameBufferInfo, nullptr, &frameBuffer));

				return VK_SUCCESS;
			}

		};


	}
}