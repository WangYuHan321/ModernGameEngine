#include "ApplicationWindow.h"

static uint32_t alignedSize(uint32_t value, uint32_t alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

ApplicationWin::ApplicationWin():
	ApplicationRaytracingBase()
{
	title = " ApplicationRayTexture ";

	m_camera.type = Camera::CameraType::lookat;
	m_camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 512.0f);
	m_camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	m_camera.setTranslation(glm::vec3(0.0f, 0.0f, -2.5f));
	EnableExtensions();

	m_enabledFeatures.shaderInt64 = VK_TRUE;
}

ApplicationWin::~ApplicationWin()
{
	vkDestroyPipeline(m_device, m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
	vkDestroyImageView(m_device, m_storageImage.view, nullptr);
	vkDestroyImage(m_device, m_storageImage.image, nullptr);
	vkFreeMemory(m_device, m_storageImage.memory, nullptr);
	vkFreeMemory(m_device, m_bottomLevelAS.memory, nullptr);
	vkDestroyBuffer(m_device, m_bottomLevelAS.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(m_device, m_bottomLevelAS.handle, nullptr);
	vkFreeMemory(m_device, m_topLevelAS.memory, nullptr);
	vkDestroyBuffer(m_device, m_topLevelAS.buffer, nullptr);
	vkDestroyAccelerationStructureKHR(m_device, m_topLevelAS.handle, nullptr);
	m_vertexBuffer.Destroy();
	m_indexBuffer.Destroy();
	m_transformBuffer.Destroy();
	for (auto& buffer : m_uniformBuffer) {
		buffer.Destroy();
	}
}

void ApplicationWin::GetEnabledFeatures()
{
	enabledBufferDeviceAddresFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	enabledBufferDeviceAddresFeatures.bufferDeviceAddress = VK_TRUE;

	enabledRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	enabledRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
	enabledRayTracingPipelineFeatures.pNext = &enabledBufferDeviceAddresFeatures;

	enabledAccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	enabledAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
	enabledAccelerationStructureFeatures.pNext = &enabledRayTracingPipelineFeatures;

	m_deviceCreatepNextChain = &enabledAccelerationStructureFeatures;
}

void ApplicationWin::CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure,
	VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &accelerationStructure.buffer));
	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(m_device, accelerationStructure.buffer, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &accelerationStructure.memory));
	VK_CHECK_RESULT(vkBindBufferMemory(m_device, accelerationStructure.buffer, accelerationStructure.memory, 0));
}

void ApplicationWin::CreateBottomLevelAccelerationStructure()
{
	struct Vertex 
	{ 
		float pos[3];
		float normal[3];
		float uv[2];
	};

	std::vector<Vertex> vertices = {
			{ {  0.5f,  0.5f, 0.0f }, {.0f, .0f, -1.0f}, { 1.0f, 1.0f} },
			{ {  -.5f,  0.5f, 0.0f }, {.0f, .0f, -1.0f}, { 0.0f, 1.0f} },
			{ {  -.5f,  -.5f, 0.0f }, {.0f, .0f, -1.0f}, { 0.0f, 0.0f} },
			{ {  0.5f,  -.5f, 0.0f }, {.0f, .0f, -1.0f}, { 1.0f, 0.0f} },
	};

	// Setup indices
	std::vector<uint32_t> indices = { 0, 1, 2, 0, 3, 2 };
	m_indexCount = static_cast<uint32_t>(indices.size());

	// Setup identity transform matrix
	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_vertexBuffer,
		vertices.size() * sizeof(Vertex),
		vertices.data()
	));

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_indexBuffer,
		indices.size() * sizeof(uint32_t),
		indices.data()
	));

	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_transformBuffer,
		sizeof(VkTransformMatrixKHR),
		&transformMatrix
	));

	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(m_vertexBuffer.buffer);
	indexBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(m_indexBuffer.buffer);
	transformBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(m_transformBuffer.buffer);

	//创建
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	//accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // 完全不透明（性能最优）
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;// 设备地址，必须是GPU可访问的内存
	accelerationStructureGeometry.geometry.triangles.maxVertex = 3; //  // 最大顶点索引// 如果有3个顶点，索引是0,1,2，则maxVertex=2
	accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
	accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
	accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
	accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
	accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
	accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;// 可选的变换矩阵，允许在构建时应用变换// 如果不需要变换，可以设为0

	//获取大小信息
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;// 优化光线追踪查询速度（牺牲构建速度）
	//VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
	// 优化构建速度（牺牲查询速度）
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	const uint32_t numTriangles = static_cast<uint32_t>(indices.size() / 3);;
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		m_device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&numTriangles,
		&accelerationStructureBuildSizesInfo);

	CreateAccelerationStructureBuffer(m_bottomLevelAS, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = m_bottomLevelAS.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(m_device, &accelerationStructureCreateInfo, nullptr, &m_bottomLevelAS.handle);

	ScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = m_bottomLevelAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	VkCommandBuffer commandBuffer = vulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	vkCmdBuildAccelerationStructuresKHR(
		commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());
	vulkanDevice->FlushCommandBuffer(commandBuffer, m_queue);

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = m_bottomLevelAS.handle;
	m_bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(m_device, &accelerationDeviceAddressInfo);

	DeleteScratchBuffer(scratchBuffer);
	/*
	
					准备几何数据
					    ↓
					配置几何描述
					    ↓
					查询内存需求
					    ↓
					    ├─ 分配加速结构内存   ScratchBuffer
					    ├─ 分配临时内存   ScratchBuffer
					    ↓
					创建加速结构对象
					    ↓
					执行GPU构建命令
					    ↓
					获取设备地址
					    ↓
					清理临时资源
	
	*/
}
/*

												方面		BLAS								TLAS
												几何类型	TRIANGLES/AABBS						INSTANCES
												数据内容	顶点/索引数据							实例引用
												实例引用	无									引用BLAS地址
												primitiveCount	三角形数量					实例数量
												构建频率	低频（几何变化时）						高频（每帧可更新）


												
												[BLAS]：一个3D模型（无论静态还是动态）
												[TLAS]：场景中这个模型的多个副本（实例）

												BLAS 存储几何数据（可能静态也可能动态更新）
												TLAS 存储实例引用和变换（跟踪物体在场景中的位置）
*/

void ApplicationWin::CreateTopLevelAccelerationStructure()
{
	VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

	VkAccelerationStructureInstanceKHR instance{};
	instance.transform = transformMatrix;
	instance.instanceCustomIndex = 0;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	instance.accelerationStructureReference = m_bottomLevelAS.deviceAddress;

	// Buffer for instance data
	Render::Vulkan::Buffer instancesBuffer;
	VK_CHECK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&instancesBuffer,
		sizeof(VkAccelerationStructureInstanceKHR),
		&instance));

	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(instancesBuffer.buffer);

	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

	// Get size info
	/*
	The pSrcAccelerationStructure, dstAccelerationStructure, and mode members of pBuildInfo are ignored. Any VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.*
	*/
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	uint32_t primitive_count = 1;

	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	vkGetAccelerationStructureBuildSizesKHR(
		m_device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		&primitive_count,
		&accelerationStructureBuildSizesInfo);

	CreateAccelerationStructureBuffer(m_topLevelAS, accelerationStructureBuildSizesInfo);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = m_topLevelAS.buffer;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	vkCreateAccelerationStructureKHR(m_device, &accelerationStructureCreateInfo, nullptr, &m_topLevelAS.handle);

	// Create a small scratch buffer used during build of the top level acceleration structure
	ScratchBuffer scratchBuffer = CreateScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = m_topLevelAS.handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = 1;
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
	VkCommandBuffer commandBuffer = vulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	vkCmdBuildAccelerationStructuresKHR(
		commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());
	vulkanDevice->FlushCommandBuffer(commandBuffer, m_queue);

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = m_topLevelAS.handle;
	m_topLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(m_device, &accelerationDeviceAddressInfo);

	DeleteScratchBuffer(scratchBuffer);
	instancesBuffer.Destroy();
}

void ApplicationWin::LoadAssets()
{
#if defined(__ANDROID__)
	m_texture.LoadFromFile("texture/floor.png", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
#else
	m_texture.LoadFromFile("./Asset/texture/floor.png", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, m_queue,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_LAYOUT_GENERAL);
#endif
}

void ApplicationWin::CreateStorageImage()
{
	VkImageCreateInfo image = Render::Vulkan::Initializer::ImageCreateInfo();
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = m_swapChain.colorFormat;
	image.extent.width = width;
	image.extent.height = height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VK_CHECK_RESULT(vkCreateImage(m_device, &image, nullptr, &m_storageImage.image));

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(m_device, m_storageImage.image, &memReqs);
	VkMemoryAllocateInfo memoryAllocateInfo = Render::Vulkan::Initializer::MemoryAllocInfo();
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = vulkanDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(m_device, &memoryAllocateInfo, nullptr, &m_storageImage.memory));
	VK_CHECK_RESULT(vkBindImageMemory(m_device, m_storageImage.image, m_storageImage.memory, 0));

	VkImageViewCreateInfo colorImageView = Render::Vulkan::Initializer::ImageViewCreateInfo();
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = m_swapChain.colorFormat;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	colorImageView.image = m_storageImage.image;
	VK_CHECK_RESULT(vkCreateImageView(m_device, &colorImageView, nullptr, &m_storageImage.view));

	VkCommandBuffer cmdBuffer = vulkanDevice->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	Render::Vulkan::Tool::SetImageLayout(cmdBuffer, m_storageImage.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	vulkanDevice->FlushCommandBuffer(cmdBuffer, m_queue);
}

void ApplicationWin::CreateUniformBuffer()
{
	for (auto& buffer : m_uniformBuffer) {
		VK_CHECK_RESULT(vulkanDevice->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, sizeof(UniformData), &m_uniformData));
		VK_CHECK_RESULT(buffer.Map());
	}
}

void ApplicationWin::CreateRayTracingPipeline()
{
	VkDescriptorSetLayoutBinding  accelerationStructureLayoutBinding{};
	accelerationStructureLayoutBinding.binding = 0;
	accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureLayoutBinding.descriptorCount = 1;
	accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
	resultImageLayoutBinding.binding = 1;
	resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	resultImageLayoutBinding.descriptorCount = 1;
	resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniformBufferBinding{};
	uniformBufferBinding.binding = 2;
	uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferBinding.descriptorCount = 1;
	uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

	VkDescriptorSetLayoutBinding ImageLayoutBinding3{};
	ImageLayoutBinding3.binding = 3;
	ImageLayoutBinding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ImageLayoutBinding3.descriptorCount = 1;
	ImageLayoutBinding3.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;



	std::vector<VkDescriptorSetLayoutBinding> bindings({
		accelerationStructureLayoutBinding,
		resultImageLayoutBinding,
		uniformBufferBinding,
		ImageLayoutBinding3
		});

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCI{};
	descriptorSetlayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetlayoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetlayoutCI.pBindings = bindings.data();
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorSetlayoutCI, nullptr, &m_descriptorSetLayout));

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(uint64_t) * 2;

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutCI.pushConstantRangeCount = 1;
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCI, nullptr, &m_pipelineLayout));

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	// Ray generation group
	{
#if defined (__ANDROID__)
		shaderStages.push_back(LoadShader("shaders/glsl/draw_rayTexture/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR));
#else
		shaderStages.push_back(LoadShader("./Asset/shader/glsl/draw_rayTexture/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR));
#endif
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		m_shaderGroups.push_back(shaderGroup);
	}

	// Miss group                            glslangValidator
	{
#if defined (__ANDROID__)
		shaderStages.push_back(LoadShader("shaders/glsl/draw_rayTexture/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
#else
		shaderStages.push_back(LoadShader("./Asset/shader/glsl/draw_rayTexture/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR));
#endif
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		m_shaderGroups.push_back(shaderGroup);
	}

	// Closest hit group
	{
#if defined (__ANDROID__)
		shaderStages.push_back(LoadShader("shaders/glsl/draw_rayTexture/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
#else
		shaderStages.push_back(LoadShader("./Asset/shader/glsl/draw_rayTexture/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
#endif
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;

#if defined (__ANDROID__)
		shaderStages.push_back(LoadShader("shaders/glsl/draw_rayTexture/anyhit.rahit.spv", VK_SHADER_STAGE_ANY_HIT_BIT_KHR));
#else
		shaderStages.push_back(LoadShader("./Asset/shader/glsl/draw_rayTexture/anyhit.rahit.spv", VK_SHADER_STAGE_ANY_HIT_BIT_KHR));
#endif

		shaderGroup.anyHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
		m_shaderGroups.push_back(shaderGroup);
	}

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	rayTracingPipelineCI.pStages = shaderStages.data();
	rayTracingPipelineCI.groupCount = static_cast<uint32_t>(m_shaderGroups.size());
	rayTracingPipelineCI.pGroups = m_shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
	rayTracingPipelineCI.layout = m_pipelineLayout;
	VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_pipeline));
}

void ApplicationWin::CreateShaderBindingTables()
{
	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize,
		rayTracingPipelineProperties.shaderGroupHandleAlignment);
	const uint32_t groupCount = static_cast<uint32_t>(m_shaderGroups.size());
	const uint32_t sbtSize = groupCount * handleSizeAligned;

	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(m_device, m_pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));
	
	CreateShaderBindingTable(m_shaderBindingTables.raygen, 1);
	CreateShaderBindingTable(m_shaderBindingTables.miss, 1);
	CreateShaderBindingTable(m_shaderBindingTables.hit, 1);

	memcpy(m_shaderBindingTables.raygen.mapped, shaderHandleStorage.data(), handleSize);
	memcpy(m_shaderBindingTables.miss.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize);
	memcpy(m_shaderBindingTables.hit.mapped, shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
}

void ApplicationWin::CreateDescriptorSets()
{
	// Pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_FRAMES_IN_FLIGHT },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT }
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = Render::Vulkan::Initializer::DescriptorPoolCreateInfo(poolSizes, MAX_FRAMES_IN_FLIGHT);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));

	// Sets per frame, just like the buffers themselves
	// Acceleration structure and storage image does not need to be duplicated per frame, we use the same for each descriptor to keep things simple
	VkDescriptorSetAllocateInfo allocInfo = Render::Vulkan::Initializer::DescriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout, 1);
	for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet[i]));

		// The fragment shader needs access to the ray tracing acceleration structure, so we pass it as a descriptor

		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &m_topLevelAS.handle;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = m_descriptorSet[i];
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = m_storageImage.view;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			// Binding 0: Top level acceleration structure
			accelerationStructureWrite,
			// Binding 1: Ray tracing result image
			Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSet[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &storageImageDescriptor),
			// Binding 2: Uniform data
			Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSet[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &m_uniformBuffer[i].descriptor),
			// Binding 3: Texture image
			Render::Vulkan::Initializer::WriteDescriptorSet(m_descriptorSet[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &m_texture.descirptor),
		};

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	}
}

VkShaderModule ApplicationWin::LoadSPIRVShader(const std::string& filename)
{
	size_t shaderSize;
	char* shaderCode{ nullptr };

#if defined (__ANDROID__)
    AAsset* asset = AAssetManager_open(androidApp->activity->assetManager,filename.c_str(), AASSET_MODE_STREAMING );
    assert(asset);
    shaderSize = AAsset_getLength(asset);
    assert(shaderSize > 0);

    shaderCode = new char[shaderSize];
    AAsset_read(asset, shaderCode, shaderSize);
    AAsset_close(asset);
#else
	std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (is.is_open())
	{
		shaderSize = is.tellg();
		is.seekg(0, std::ios::beg);
		// Copy file contents into a buffer
		shaderCode = new char[shaderSize];
		is.read(shaderCode, shaderSize);
		is.close();
		assert(shaderSize > 0);
	}
#endif
	if (shaderCode)
	{
		// Create a new shader module that will be used for pipeline creation
		VkShaderModuleCreateInfo shaderModuleCI{};
		shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCI.codeSize = shaderSize;
		shaderModuleCI.pCode = (uint32_t*)shaderCode;

		VkShaderModule shaderModule;
		VK_CHECK_RESULT(vkCreateShaderModule(m_device, &shaderModuleCI, nullptr, &shaderModule));

		delete[] shaderCode;

		return shaderModule;
	}
	else
	{
		std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
		return VK_NULL_HANDLE;
	}
}


void ApplicationWin::Prepare() 
{
	ApplicationRaytracingBase::Prepare();
	LoadAssets();

	CreateBottomLevelAccelerationStructure();
	CreateTopLevelAccelerationStructure();
	CreateStorageImage();
	CreateUniformBuffer();

	CreateRayTracingPipeline();
	CreateShaderBindingTables();
	CreateDescriptorSets();

	prepared = true;
}

void ApplicationWin::UpdateUniformBuffers()
{
	m_uniformData.projInverse = glm::inverse(m_camera.matrices.perspective);
	m_uniformData.viewInverse = glm::inverse(m_camera.matrices.view);
	memcpy(m_uniformBuffer[m_currentBuffer].mapped, &m_uniformData, sizeof(UniformData));
}

void ApplicationWin::BuildCommandBuffer()
{
	VkCommandBuffer cmdBuffer = m_drawCmdBuffers[m_currentBuffer];

	VkCommandBufferBeginInfo cmdBufInfo = Render::Vulkan::Initializer::CommandBufferBeginInfo();

	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipeline);

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_pipelineLayout, 0, 1, &m_descriptorSet[m_currentBuffer], 0, 0);

	struct BufferReferences {
		uint64_t vertices;
		uint64_t indices;
	} bufferReferences{};

	bufferReferences.vertices = GetBufferDeviceAddress(m_vertexBuffer.buffer);
	bufferReferences.indices = GetBufferDeviceAddress(m_indexBuffer.buffer);

	vkCmdPushConstants(cmdBuffer, m_pipelineLayout, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
		VK_SHADER_STAGE_ANY_HIT_BIT_KHR, 0, sizeof(uint64_t) * 2, &bufferReferences);

	VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

	vkCmdTraceRaysKHR(
		cmdBuffer,
		&m_shaderBindingTables.raygen.stridedDeviceAddressRegion,
		&m_shaderBindingTables.miss.stridedDeviceAddressRegion,
		&m_shaderBindingTables.hit.stridedDeviceAddressRegion,
		&callableShaderSbtEntry,
		width, height, 1
	);

	Render::Vulkan::Tool::SetImageLayout(
		cmdBuffer,
		m_swapChain.images[m_currentImageIndex],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	Render::Vulkan::Tool::SetImageLayout(
		cmdBuffer,
		m_storageImage.image,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		subresourceRange);

	VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent = { width, height, 1 };
	vkCmdCopyImage(cmdBuffer, m_storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
		m_swapChain.images[m_currentImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);


	Render::Vulkan::Tool::SetImageLayout(
		cmdBuffer,
		m_swapChain.images[m_currentImageIndex],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		subresourceRange);

	Render::Vulkan::Tool::SetImageLayout(
		cmdBuffer,
		m_storageImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		subresourceRange);

	DrawUI(cmdBuffer, m_frameBuffers[m_currentImageIndex]);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
}

void ApplicationWin::Render()
{
	if (!prepared)
		return;

	VK_CHECK_RESULT(vkWaitForFences(m_device, 1, &m_waitFences[m_currentBuffer], VK_TRUE, UINT64_MAX));
	VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_waitFences[m_currentBuffer]));
	m_swapChain.AcquireNextImage(m_presentCompleteSemaphores[m_currentBuffer], m_currentImageIndex);

	UpdateUniformBuffers();
	BuildCommandBuffer();

	ApplicationBase::SubmitFrame(false);

	vkQueueWaitIdle(m_queue);
}