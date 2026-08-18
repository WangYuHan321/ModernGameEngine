#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "Instance/VResourceManager.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	class VRenderPass final
	{
	//types
	private:
		static constexpr uint maxColorAttachment = GFG_MaxColorBuffers;
		static constexpr uint maxAttachments = GFG_MaxColorBuffers + 1;
		static constexpr uint maxSubPass = GFG_MaxRenderPassSubpasses;
		static constexpr uint maxDependencies = maxSubPass * 2;

		using Attachments_t = FixedArray<VkAttachmentDescription, maxAttachments>;
		using AttachmentsRef_t = FixedArray<VkAttachmentReference, maxAttachments* maxSubPass>;
		using AttachmentsRef2_t = FixedArray<VkAttachmentReference, maxSubPass>;
		using SubPasses_t = FixedArray<VkSubpassDescription, maxSubPass>;
		using Dependencies_t = FixedArray<VkSubpassDependency, maxDependencies>;






	};







}

