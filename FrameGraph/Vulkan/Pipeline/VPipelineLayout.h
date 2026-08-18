#pragma once
#include "../Public/FrameGraph.h"
#include "../Shared/LocalResourceID.h"
#include "Instance/VResourceManager.h"
#include "../STL/Common.h"
#include "vulkan/vulkan.h"

namespace FrameGraph
{
	class VPipelineLayout final
	{
	//type
	private:
		struct DescSetLayout
		{
			RawDescriptorSetLayoutID layoutID;
			VkDescriptorSetLayout     layout;
			uint index;

			DescSetLayout() {}
			DescSetLayout(RawDescriptorSetLayoutID id, VkDescriptorSetLayout layout, uint index) :layoutID(id), layout(layout), index(index) {}
		};

		static constexpr uint MaxDescSets = GFG_MaxDescriptorSets;

		using DescriptorSets_t = FixedMap<DescriptorSetID, DescSetLayout, MaxDescSets>;
		using PushConstants_t = PipelineDescription::PushConstants_t;
		using VkDescriptorSetLayout_t = StaticArray<VkDescriptorSetLayout, MaxDescSets>;
		using VkPushConstantRange_t = FixedArray<VkDescriptorSetLayout, GFG_MaxPushConstants>;
		using DSLayoutArray_t = ArrayView<Pair<RawDescriptorSetLayoutID, ResourceBase<VkDescriptorSetLayout>*>>;


	private:
		HashVal _hash;
		VkPipelineLayout _layout;
		DescriptorSets_t _descriptorSets;
		PushConstants_t _pushConstants;
		uint _firstDescSet = UMax;

		DebugName_t _debugName;
		RWDataRaceCheck _drCheck;

	public:
		VPipelineLayout() {}
		VPipelineLayout(VPipelineLayout&&) = delete;
		VPipelineLayout(const VPipelineLayout&) = delete;
		VPipelineLayout(const PipelineDescription::PipelineLayout& ppln, DSLayoutArray_t sets);
		~VPipelineLayout();

		bool Create(const VDevice& dev, VkDescriptorSetLayout emptyLayout);
		void Destroy(VResourceManager&);


	};

}

