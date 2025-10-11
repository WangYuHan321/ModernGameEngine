#pragma once
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // 深度缓存区，OpenGL默认是（-1， 1）Vulakn为（0.0， 1.0）
#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <array>


#define VIEWPORT_VIEW 1080
#define VIEWPORT_HEIGHT 720
#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1


struct InstanceData
{
    glm::vec3 InstancePosition;
    glm::vec3 InstanceRotation;
    float InstancePScale;
    glm::uint8 InstanceTexIndex;
};

/** 物体的MVP矩阵信息*/
struct UniformBufferObject {
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Proj;
};

/** 顶点数据存储结构*/
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Color;
    glm::vec2 TexCoord;

    // 顶点描述
    static VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = VERTEX_BUFFER_BIND_ID;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Normal);

        attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, Color);

        attributeDescriptions[3].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, TexCoord);

        return attributeDescriptions;
    }

    // 顶点描述，带Instance
    static std::array<VkVertexInputBindingDescription, 2> GetBindingInstancedDescriptions() {
        VkVertexInputBindingDescription bindingDescription0{};
        bindingDescription0.binding = VERTEX_BUFFER_BIND_ID;
        bindingDescription0.stride = sizeof(Vertex);
        bindingDescription0.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputBindingDescription bindingDescription1{};
        bindingDescription1.binding = INSTANCE_BUFFER_BIND_ID;
        bindingDescription1.stride = sizeof(InstanceData);
        bindingDescription1.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return { bindingDescription0, bindingDescription1 };
    }

    static std::array<VkVertexInputAttributeDescription, 8> GetAttributeInstancedDescriptions() {
        std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions{};

        attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Normal);

        attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, Color);

        attributeDescriptions[3].binding = VERTEX_BUFFER_BIND_ID;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, TexCoord);

        attributeDescriptions[4].binding = INSTANCE_BUFFER_BIND_ID;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(InstanceData, InstancePosition);

        attributeDescriptions[5].binding = INSTANCE_BUFFER_BIND_ID;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(InstanceData, InstanceRotation);

        attributeDescriptions[6].binding = INSTANCE_BUFFER_BIND_ID;
        attributeDescriptions[6].location = 6;
        attributeDescriptions[6].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[6].offset = offsetof(InstanceData, InstancePScale);

        attributeDescriptions[7].binding = INSTANCE_BUFFER_BIND_ID;
        attributeDescriptions[7].location = 7;
        attributeDescriptions[7].format = VK_FORMAT_R8_UINT;
        attributeDescriptions[7].offset = offsetof(InstanceData, InstanceTexIndex);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return Position == other.Position && Normal == other.Normal && Color == other.Color && TexCoord == other.TexCoord;
    }
};

struct InstanceMesh
{
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    VkBuffer VertexBuffer;
    VkDeviceMemory VertexBufferMemory;                
    VkBuffer IndexBuffer;                              
    VkDeviceMemory IndexBufferMemory;                  

    // only init with instanced mesh
    VkBuffer InstancedBuffer;                            // Instanced buffer
    VkDeviceMemory InstancedBufferMemory;                // Instanced buffer memory
};

struct Mesh
{
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    VkBuffer VertexBuffer;
    VkDeviceMemory VertexBufferMemory;
    VkBuffer IndexBuffer;
    VkDeviceMemory IndexBufferMemory;
};

struct Material
{
    std::vector<VkImage> TextureImages;                  
    std::vector<VkDeviceMemory> TextureImageMemorys;     
    std::vector<VkImageView> TextureImageViews;         
    std::vector<VkSampler> TextureSamplers;  

    VkDescriptorPool DescriptorPool;
    std::vector<VkDescriptorSet> DescriptorSets;
};

struct RenderInstanceObject 
{
    Material mat;
    InstanceMesh MeshData;
    uint32_t InstanceCount;
};

struct RenderObject
{
    Material mat;
    Mesh meshData;
};

struct RenderIndirectObject
{
    VkBuffer IndirectBuffer;                            // Indirect buffer
    VkDeviceMemory IndirectBufferMemory;                // Indirect buffer memory
    std::vector<VkDrawIndexedIndirectCommand> IndirectCommands;
};

struct RenderIndirectInstancedObject
{
    VkBuffer IndirectBuffer;                            // Indirect buffer
    VkDeviceMemory IndirectBufferMemory;                // Indirect buffer memory
    std::vector<VkDrawIndexedIndirectCommand> IndirectCommands;
    uint32_t InstanceCount;
};

const int MAX_FRAMES_IN_FLIGHT = 2;

struct GeometryBuffer
{
    //depth
    VkFormat DepthStencilFormat;
	VkImage DepthStencilImage;
	VkDeviceMemory DepthStencilMemory;
	VkImageView DepthStencilView;
	VkSampler DepthStencilSampler;
    
    //SceneCOlor
    VkFormat SceneColorFormat;
    VkImage SceneColorImage;
    VkDeviceMemory SceneColorMemory;
    VkImageView SceneColorImageView;
    VkSampler SceneColorSampler;

	//GBUFFER Normal
    VkFormat GBufferAFormat;
    VkImage GBufferAImage;
    VkDeviceMemory GBufferAMemory;
    VkImageView GBufferAImageView;
    VkSampler GBufferASampler;


    // Metallic + Roughness + Reflect
    VkFormat GBufferBFormat;
    VkImage GBufferBImage;
    VkDeviceMemory GBufferBMemory;
    VkImageView GBufferBImageView;
    VkSampler GBufferBSampler;
    
    // BaseColor+AO
    VkFormat GBufferCFormat;
    VkImage GBufferCImage;
    VkDeviceMemory GBufferCMemory;
    VkImageView GBufferCImageView;
    VkSampler GBufferCSampler;

    // Position+ID
    VkFormat GBufferDFormat;
    VkImage GBufferDImage;
    VkDeviceMemory GBufferDMemory;
    VkImageView GBufferDImageView;
    VkSampler GBufferDSampler;
};

struct ShadowPass
{
    std::vector<RenderObject*> renderObjects;
	std::vector<RenderInstanceObject*> renderInstanceObjects;
	std::vector<RenderIndirectObject*> renderIndirectObjects;
	std::vector<RenderIndirectInstancedObject*> renderIndirectInstancedObjects;

    float zNear, zFar;
    int32_t Width, Height;
    VkFormat Format;
	VkFramebuffer FrameBuffer;
    VkRenderPass RenderPass;
    VkImage Image;
    VkDeviceMemory Memory;
    VkImageView ImageView;
    VkSampler Sampler;
	VkDescriptorSetLayout DescriptionSetLayout;
	VkDescriptorPool DescriptorPool;
	VkPipelineLayout PipelineLayout;
	VkPipeline Pipeline;
    VkPipeline PipelineInstanced;
    std::vector<VkDescriptorSet> DescriptorSets;
    std::vector<VkBuffer> UniformBuffers;
	std::vector<VkDeviceMemory> UniformBuffersMemory;
};

class BackgroundPass
{
    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory Memory;
    VkSampler Sampler;
    VkRenderPass RenderPass;
	VkDescriptorSetLayout DescriptorSetLayout;
    VkDescriptorPool DescriptorPool;
	std::vector<VkDescriptorSet> DescriptorSets;
    VkPipelineLayout PipelineLayout;
	VkPipeline PipelineInstanced;
	std::vector<VkPipeline> Pipelines;
} ;

class SkyboxPass
{
    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory Memory;
    VkSampler Sampler;
    VkRenderPass RenderPass;
    VkDescriptorSetLayout DescriptorSetLayout;
    VkDescriptorPool DescriptorPool;
    std::vector<VkDescriptorSet> DescriptorSets;
    VkPipelineLayout PipelineLayout;
    VkPipeline PipelineInstanced;
    std::vector<VkPipeline> Pipelines;
};

struct BaseScenePass
{
    std::vector<RenderObject> RenderObjects;
    std::vector<RenderInstanceObject> RenderInstanceObjects;
	VkDescriptorSetLayout DescriptorSetLayout;
	VkPipelineLayout PipelineLayout;
	std::vector<VkPipeline> Pipelines;
	std::vector<VkPipeline> PipelinesInstanced;
};

struct BaseSceneIndirectPass
{
    std::vector<RenderIndirectObject> RenderIndirectObjects;
    std::vector<RenderIndirectInstancedObject> RenderIndirectInstanceObjects;
    VkDescriptorSetLayout DescriptorSetLayout;
    VkPipelineLayout PipelineLayout;
    std::vector<VkPipeline> Pipelines;
    std::vector<VkPipeline> PipelinesInstanced;
};

struct BaseSceneDeferredRenderingPass
{
    std::vector<RenderObject> RenderObjects;					// 待渲染的物体
    std::vector<RenderInstanceObject> RenderInstanceObjects;	// 待渲染的物体
    VkDescriptorSetLayout SceneDescriptorSetLayout;				// 描述符集合布局
    VkPipelineLayout ScenePipelineLayout;						// 渲染管线布局
    std::vector<VkPipeline> ScenePipelines;						// 渲染管线
    std::vector<VkPipeline> ScenePipelinesInstanced;			// 渲染管线
    VkFramebuffer SceneFrameBuffer;
    VkRenderPass SceneRenderPass;
    VkDescriptorSetLayout LightingDescriptorSetLayout;
    VkDescriptorPool LightingDescriptorPool;
    std::vector<VkDescriptorSet> LightingDescriptorSets;
    VkPipelineLayout LightingPipelineLayout;
    std::vector<VkPipeline> LightingPipelines;
};

class RHIVulkan
{
public:

    RHIVulkan();
    ~RHIVulkan();

    void InitWindow();
    void InitVulkan();
    void MainLoop();

private:

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation" //  VK_LAYER_LUNARG_standard_validation
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
        //"VK_KHR_portability_subset"
        //VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

private:
    VkInstance m_vkInstance;
    VkDevice m_vkDevice;
    VkDebugUtilsMessengerEXT m_debugMessenger; //Debug

	RenderObject m_renderObject;
    ShadowPass m_shadowmapPass;

    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_vkImageAvailableSemaphores;    // 图像是否完成的信号
    std::vector<VkSemaphore> m_vkRenderFinishedSemaphores;    // 渲染是否结束的信号
    std::vector<VkFence> m_vkInFlightFences;                // 围栏，下一帧渲染前等待上一帧全部渲染完成

    VkSwapchainKHR m_vkSwapChain;
    std::vector<VkImage> m_vkSwapChainImages;
    VkFormat m_vkSwapChainImageFormat;
    VkExtent2D m_vkSwapChainExtent;
    std::vector<VkImageView> m_vkSwapChainImageViews;
    std::vector<VkFramebuffer> m_vkSwapChainFrameBuffers;

    VkRenderPass m_vkRenderPass;
    VkPipelineLayout m_vkPipeLineLayout;
    VkPipeline m_vkGraphicsPipeline;

    GLFWwindow* m_glfwWindow;
    VkSurfaceKHR m_surfaceWindow;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    std::vector<VkCommandBuffer> m_vkCommandBuffer;
    VkCommandPool m_vkCommandPool;

    uint32_t m_windowWidth = 800;
    uint32_t m_windowHeight = 600;
    uint32_t m_currentFrame = 0;

    VkBuffer m_vertBuffers;
    VkDeviceMemory m_vertBufferMemory;

    VkBuffer m_indexBuffers;
    VkDeviceMemory m_indexBufferMemory;

    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
    
    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;

    VkDescriptorSetLayout m_vkDescriptorSetLayout;			// 描述符集合配置，在渲染管线创建时指定
    VkDescriptorPool m_vkDescriptorPool;					// 描述符池，存放描述符
    std::vector<VkDescriptorSet> m_vkDescriptorSets;		// 描述符集合，描述符使得着色器可以自由的访问缓存和图片

    std::vector<VkBuffer> m_vkUniformBuffers;				// 统一缓存区
    std::vector<VkDeviceMemory> m_vkUniformBuffersMemory;	// 统一缓存区内存地址
private:

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    std::vector<const char*> GetRequiredExtensions();
    void PopulateDebugMessagerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool CheckValidationLayerSupport();
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);

    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void CreateInstance();
    void SetupDebugMessage();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void ReCreateSwapChain();
    void CleanSwapChain();
    void CreateImageViews();
    void ReadModelResource();
    void CreateUniformBuffer();
    void CreateDescriptorSets();
    void CreateDescriptorPool();
    void CreateRenderPass();
    void CreateDepthResource();
    void CreateGraphicsPipeline();
    void CreateFrameBuffer();
    void CreateCommandPool();
    void CreateDescriptorSetLayout();

    void CreateGeometry(std::vector<Vertex>&outVert, std::vector<uint32_t>& outIndice, const std::string& fileName);
    void CreateShadowmapPass();
	void CreateBaseScenePass();

    void UpdateUniformBuffer(uint32_t currentImage);

    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateSampler(
        VkSampler& outSampler,
        const VkFilter filter = VK_FILTER_LINEAR,
        const VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        const VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        const VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        const VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        const uint32_t miplevels = 1);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


    void CreateVertexBuffer(VkBuffer& outBuffer, VkDeviceMemory& outMemory, const std::vector<Vertex>& inVertex);
    void CreateIndexBuffer(VkBuffer& outBuffer, VkDeviceMemory& outMemory, const std::vector<uint32_t>& inIndices);

    VkFormat FindDepthFormat();
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void DrawFrame();
    void Cleanup();
};
