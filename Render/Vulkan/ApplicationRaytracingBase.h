#pragma once

#include"ApplicationBase.h"

class ApplicationRaytracingBase : ApplicationBase
{
protected:
	virtual void SetupRenderPass();
	virtual void SetupFrameBuffer();

public:
	// Function pointers for ray tracing related stuff
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR{ nullptr };
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{ nullptr };
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{ nullptr };
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{ nullptr };
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{ nullptr };
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR{ nullptr };
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{ nullptr };
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR{ nullptr };
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR{ nullptr };
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR{ nullptr };

	// Available features and properties
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

	// Enabled features and properties
	VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};

	struct ScratchBuffer
	{
		uint64_t deviceAddress{ 0 };
		VkBuffer handle{ VK_NULL_HANDLE };
		VkDeviceMemory memory{ VK_NULL_HANDLE };
	};

	struct AccelerationStructure
	{
		VkAccelerationStructureKHR handle{ VK_NULL_HANDLE };
		uint64_t deviceAddress{ 0 };
		VkBuffer buffer{ VK_NULL_HANDLE };
		VkDeviceMemory memory{ VK_NULL_HANDLE };
	};

	struct StorageImage
	{
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkImage image{ VK_NULL_HANDLE };
		VkImageView view{ VK_NULL_HANDLE };
		VkFormat format;
	};

	class ShaderBindingTable :public Render::Vulkan::Buffer
	{
	public:
		VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
	};

	bool rayQueryOnly = false;

	StorageImage m_storageImage;

	void EnableExtensions();
	ScratchBuffer CreateScratchBuffer(VkDeviceSize size);
	void DeleteScratchBuffer(ScratchBuffer& scratchBuffer);
	void CreateAccelerationStructure(AccelerationStructure& accelerationStructure, 
		VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
	void DeleteAccelerationStructure(AccelerationStructure& accelerationStructure);
	uint64_t GetBufferDeviceAddress(VkBuffer buffer);
	void CreateStorageImage(VkFormat format, VkExtent3D extent);
	void DeleteStorageImage();
	VkStridedDeviceAddressRegionKHR GetSbtEntryStridedDeviceAddressRegion(VkBuffer buffer, uint32_t handleCount);
	void CreateShaderBindingTable(ShaderBindingTable& shaderBindingTable, uint32_t handleCount);
	void DrawUI(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer);

	virtual void Prepare();
};
