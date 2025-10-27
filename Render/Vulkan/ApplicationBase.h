#pragma once

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingAPI.h>
#include <GLFW/glfw3.h>
#endif

#include "VulkanSwapChain.h"
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // 深度缓存区，OpenGL默认是（-1， 1）Vulakn为（0.0， 1.0）
#define STB_IMAGE_IMPLEMENTATION
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>
#include "UIOverlay.h"
#include "VulkaneDebug.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "VulkanInitializers.hpp"

#include "../Camera.hpp"

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
using namespace Render::Vulkan;


#define VIEWPORT_VIEW 1080
#define VIEWPORT_HEIGHT 720
#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1


//struct InstanceData
//{
//    glm::vec3 InstancePosition;
//    glm::vec3 InstanceRotation;
//    float InstancePScale;
//    glm::uint8 InstanceTexIndex;
//};
//
///** 物体的MVP矩阵信息*/
//struct UniformBufferObject {
//    glm::mat4 Model;
//    glm::mat4 View;
//    glm::mat4 Proj;
//};
//
struct DepthStencil
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
};
//
//struct Light
//{
//    glm::vec4 position;
//    glm::vec4 color; // rgb for color, a for intensity
//    glm::vec4 direction;
//    glm::vec4 info;
//};
//
//struct UniformBufferView {
//    glm::mat4 shadowmap_space;
//    glm::mat4 local_to_world;
//    glm::vec4 camera_info;
//    Light directional_lights[4];
//    Light point_lights[4];
//    Light spot_lights[4];
//    glm::ivec4 lights_count;
//    float zNear;
//    float zFar;
//};
//
///** 顶点数据存储结构*/
//struct Vertex {
//    glm::vec3 Position;
//    glm::vec3 Normal;
//    glm::vec3 Color;
//    glm::vec2 TexCoord;
//
//    // 顶点描述
//    static VkVertexInputBindingDescription GetBindingDescription() {
//        VkVertexInputBindingDescription bindingDescription{};
//        bindingDescription.binding = VERTEX_BUFFER_BIND_ID;
//        bindingDescription.stride = sizeof(Vertex);
//        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//        return bindingDescription;
//    }
//
//    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
//        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
//
//        attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[0].location = 0;
//        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[0].offset = offsetof(Vertex, Position);
//
//        attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[1].location = 1;
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = offsetof(Vertex, Normal);
//
//        attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[2].location = 2;
//        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[2].offset = offsetof(Vertex, Color);
//
//        attributeDescriptions[3].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[3].location = 3;
//        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
//        attributeDescriptions[3].offset = offsetof(Vertex, TexCoord);
//
//        return attributeDescriptions;
//    }
//
//    // 顶点描述，带Instance
//    static std::array<VkVertexInputBindingDescription, 2> GetBindingInstancedDescriptions() {
//        VkVertexInputBindingDescription bindingDescription0{};
//        bindingDescription0.binding = VERTEX_BUFFER_BIND_ID;
//        bindingDescription0.stride = sizeof(Vertex);
//        bindingDescription0.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//        VkVertexInputBindingDescription bindingDescription1{};
//        bindingDescription1.binding = INSTANCE_BUFFER_BIND_ID;
//        bindingDescription1.stride = sizeof(InstanceData);
//        bindingDescription1.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
//
//        return { bindingDescription0, bindingDescription1 };
//    }
//
//    static std::array<VkVertexInputAttributeDescription, 8> GetAttributeInstancedDescriptions() {
//        std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions{};
//
//        attributeDescriptions[0].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[0].location = 0;
//        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[0].offset = offsetof(Vertex, Position);
//
//        attributeDescriptions[1].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[1].location = 1;
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = offsetof(Vertex, Normal);
//
//        attributeDescriptions[2].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[2].location = 2;
//        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[2].offset = offsetof(Vertex, Color);
//
//        attributeDescriptions[3].binding = VERTEX_BUFFER_BIND_ID;
//        attributeDescriptions[3].location = 3;
//        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
//        attributeDescriptions[3].offset = offsetof(Vertex, TexCoord);
//
//        attributeDescriptions[4].binding = INSTANCE_BUFFER_BIND_ID;
//        attributeDescriptions[4].location = 4;
//        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[4].offset = offsetof(InstanceData, InstancePosition);
//
//        attributeDescriptions[5].binding = INSTANCE_BUFFER_BIND_ID;
//        attributeDescriptions[5].location = 5;
//        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[5].offset = offsetof(InstanceData, InstanceRotation);
//
//        attributeDescriptions[6].binding = INSTANCE_BUFFER_BIND_ID;
//        attributeDescriptions[6].location = 6;
//        attributeDescriptions[6].format = VK_FORMAT_R32_SFLOAT;
//        attributeDescriptions[6].offset = offsetof(InstanceData, InstancePScale);
//
//        attributeDescriptions[7].binding = INSTANCE_BUFFER_BIND_ID;
//        attributeDescriptions[7].location = 7;
//        attributeDescriptions[7].format = VK_FORMAT_R8_UINT;
//        attributeDescriptions[7].offset = offsetof(InstanceData, InstanceTexIndex);
//
//        return attributeDescriptions;
//    }
//
//    bool operator==(const Vertex& other) const {
//        return Position == other.Position && Normal == other.Normal && Color == other.Color && TexCoord == other.TexCoord;
//    }
//};
//
//struct InstanceMesh
//{
//    std::vector<Vertex> Vertices;
//    std::vector<uint32_t> Indices;
//    VkBuffer VertexBuffer;
//    VkDeviceMemory VertexBufferMemory;                
//    VkBuffer IndexBuffer;                              
//    VkDeviceMemory IndexBufferMemory;                  
//
//    // only init with instanced mesh
//    VkBuffer InstancedBuffer;                            // Instanced buffer
//    VkDeviceMemory InstancedBufferMemory;                // Instanced buffer memory
//};
//
//struct Mesh
//{
//    std::vector<Vertex> Vertices;
//    std::vector<uint32_t> Indices;
//    VkBuffer VertexBuffer;
//    VkDeviceMemory VertexBufferMemory;
//    VkBuffer IndexBuffer;
//    VkDeviceMemory IndexBufferMemory;
//};
//
//struct Material
//{
//    std::vector<VkImage> TextureImages;                  
//    std::vector<VkDeviceMemory> TextureImageMemorys;     
//    std::vector<VkImageView> TextureImageViews;         
//    std::vector<VkSampler> TextureSamplers;  
//
//    VkDescriptorPool DescriptorPool;
//    std::vector<VkDescriptorSet> DescriptorSets;
//};
//
//struct RenderInstanceObject 
//{
//    Material mat;
//    InstanceMesh MeshData;
//    uint32_t InstanceCount;
//};
//
//struct RenderObject
//{
//    Material mat;
//    Mesh meshData;
//};
//
//struct GlobalConstants {
//    float time;
//    float metallic;
//    float roughness;
//    uint32_t specConstants;
//    uint32_t specConstantsCount;				// 特殊常数，用于优化着色器变体
//
//    void resetConstants()
//    {
//        time = 0.0f;
//        metallic = 0.0f;
//        roughness = 1.0;
//        specConstants = 0;
//        specConstantsCount = 9;
//    }
//} ;
//
//struct RenderIndirectObject
//{
//    VkBuffer IndirectBuffer;                            // Indirect buffer
//    VkDeviceMemory IndirectBufferMemory;                // Indirect buffer memory
//    std::vector<VkDrawIndexedIndirectCommand> IndirectCommands;
//};
//
//struct RenderIndirectInstancedObject
//{
//    VkBuffer IndirectBuffer;                            // Indirect buffer
//    VkDeviceMemory IndirectBufferMemory;                // Indirect buffer memory
//    std::vector<VkDrawIndexedIndirectCommand> IndirectCommands;
//    uint32_t InstanceCount;
//};
//
const int MAX_FRAMES_IN_FLIGHT = 2;
//
//struct GeometryBuffer
//{
//    //depth
//    VkFormat DepthStencilFormat;
//	VkImage DepthStencilImage;
//	VkDeviceMemory DepthStencilMemory;
//	VkImageView DepthStencilView;
//	VkSampler DepthStencilSampler;
//    
//    //SceneCOlor
//    VkFormat SceneColorFormat;
//    VkImage SceneColorImage;
//    VkDeviceMemory SceneColorMemory;
//    VkImageView SceneColorImageView;
//    VkSampler SceneColorSampler;
//
//	//GBUFFER Normal
//    VkFormat GBufferAFormat;
//    VkImage GBufferAImage;
//    VkDeviceMemory GBufferAMemory;
//    VkImageView GBufferAImageView;
//    VkSampler GBufferASampler;
//
//
//    // Metallic + Roughness + Reflect
//    VkFormat GBufferBFormat;
//    VkImage GBufferBImage;
//    VkDeviceMemory GBufferBMemory;
//    VkImageView GBufferBImageView;
//    VkSampler GBufferBSampler;
//    
//    // BaseColor+AO
//    VkFormat GBufferCFormat;
//    VkImage GBufferCImage;
//    VkDeviceMemory GBufferCMemory;
//    VkImageView GBufferCImageView;
//    VkSampler GBufferCSampler;
//
//    // Position+ID
//    VkFormat GBufferDFormat;
//    VkImage GBufferDImage;
//    VkDeviceMemory GBufferDMemory;
//    VkImageView GBufferDImageView;
//    VkSampler GBufferDSampler;
//};
//
//struct ShadowPass
//{
//    std::vector<RenderObject*> renderObjects;
//	std::vector<RenderInstanceObject*> renderInstanceObjects;
//	std::vector<RenderIndirectObject*> renderIndirectObjects;
//	std::vector<RenderIndirectInstancedObject*> renderIndirectInstancedObjects;
//
//    float zNear, zFar;
//    int32_t Width, Height;
//    VkFormat Format;
//	VkFramebuffer FrameBuffer;
//    VkRenderPass RenderPass;
//    VkImage Image;
//    VkDeviceMemory Memory;
//    VkImageView ImageView;
//    VkSampler Sampler;
//	VkDescriptorSetLayout DescriptionSetLayout;
//	VkDescriptorPool DescriptorPool;
//	VkPipelineLayout PipelineLayout;
//	VkPipeline Pipeline;
//    VkPipeline PipelineInstanced;
//    std::vector<VkDescriptorSet> DescriptorSets;
//    std::vector<VkBuffer> UniformBuffers;
//	std::vector<VkDeviceMemory> UniformBuffersMemory;
//};
//
//class BackgroundPass
//{
//    VkImage Image;
//    VkImageView ImageView;
//    VkDeviceMemory Memory;
//    VkSampler Sampler;
//    VkRenderPass RenderPass;
//	VkDescriptorSetLayout DescriptorSetLayout;
//    VkDescriptorPool DescriptorPool;
//	std::vector<VkDescriptorSet> DescriptorSets;
//    VkPipelineLayout PipelineLayout;
//	VkPipeline PipelineInstanced;
//	std::vector<VkPipeline> Pipelines;
//} ;
//
//class SkyboxPass
//{
//    VkImage Image;
//    VkImageView ImageView;
//    VkDeviceMemory Memory;
//    VkSampler Sampler;
//    VkRenderPass RenderPass;
//    VkDescriptorSetLayout DescriptorSetLayout;
//    VkDescriptorPool DescriptorPool;
//    std::vector<VkDescriptorSet> DescriptorSets;
//    VkPipelineLayout PipelineLayout;
//    VkPipeline PipelineInstanced;
//    std::vector<VkPipeline> Pipelines;
//};
//
//struct BaseScenePass
//{
//    std::vector<RenderObject> RenderObjects;
//    std::vector<RenderInstanceObject> RenderInstanceObjects;
//	VkDescriptorSetLayout DescriptorSetLayout;
//	VkPipelineLayout PipelineLayout;
//	std::vector<VkPipeline> Pipelines;
//	std::vector<VkPipeline> PipelinesInstanced;
//};
//
//struct BaseSceneIndirectPass
//{
//    std::vector<RenderIndirectObject> RenderIndirectObjects;
//    std::vector<RenderIndirectInstancedObject> RenderIndirectInstanceObjects;
//    VkDescriptorSetLayout DescriptorSetLayout;
//    VkPipelineLayout PipelineLayout;
//    std::vector<VkPipeline> Pipelines;
//    std::vector<VkPipeline> PipelinesInstanced;
//};
//
//struct BaseSceneDeferredRenderingPass
//{
//    std::vector<RenderObject> RenderObjects;					// 待渲染的物体
//    std::vector<RenderInstanceObject> RenderInstanceObjects;	// 待渲染的物体
//    VkDescriptorSetLayout SceneDescriptorSetLayout;				// 描述符集合布局
//    VkPipelineLayout ScenePipelineLayout;						// 渲染管线布局
//    std::vector<VkPipeline> ScenePipelines;						// 渲染管线
//    std::vector<VkPipeline> ScenePipelinesInstanced;			// 渲染管线
//    VkFramebuffer SceneFrameBuffer;
//    VkRenderPass SceneRenderPass;
//    VkDescriptorSetLayout LightingDescriptorSetLayout;
//    VkDescriptorPool LightingDescriptorPool;
//    std::vector<VkDescriptorSet> LightingDescriptorSets;
//    VkPipelineLayout LightingPipelineLayout;
//    std::vector<VkPipeline> LightingPipelines;
//};

class ApplicationBase
{
protected:
    VkInstance m_instance{ VK_NULL_HANDLE };
    std::vector<std::string> m_supportedInstanceExtensions;
    VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
    VkPhysicalDeviceProperties m_deviceProperties{};
    VkPhysicalDeviceFeatures m_deviceFeatures{};
    VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties{};
    VkPhysicalDeviceFeatures m_enabledFeatures{};
    std::vector<const char*> m_enabledDeviceExtensions;
    std::vector<const char*> m_enabledInstanceExtensions;
    std::vector<VkLayerSettingEXT> m_enabledLayerSettings;

    VkDevice m_device{ VK_NULL_HANDLE };
    VkQueue m_queue{ VK_NULL_HANDLE };
    VkFormat m_depthFormat{ VK_FORMAT_UNDEFINED };
    VkCommandPool m_cmdPool{ VK_NULL_HANDLE };
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_drawCmdBuffers;
    std::vector<VkFramebuffer> m_frameBuffers;
    VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };
    std::vector<VkShaderModule> m_shaderModules;
    VkPipelineCache m_pipelineCache{ VK_NULL_HANDLE };
    VkRenderPass m_renderPass{ VK_NULL_HANDLE };
    VulkanSwapChain m_swapChain;
    DepthStencil m_depthStencil;

    uint32_t m_currentImageIndex{ 0 };
    uint32_t m_currentBuffer{ 0 };
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_presentCompleteSemaphores{};
    std::vector<VkSemaphore> m_renderCompleteSemaphores{};
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_waitFences;

    Camera m_camera;

protected:
    void CreateSurface();
    void CreateCommandPool();
    void CreateSwapChain();
    void CreatePipelineCache();
    void CreateCommandBuffers();
    void CreateSynchronizationPrimitives();
    virtual void SetupDepthStencil();
    virtual void SetupRenderPass();
    virtual void SetupFrameBuffer();
    virtual void BuildCommandBuffer();
    virtual void OnUpdateUIOverlay(UIOverlay* ui);

    void NextFrame();
    void UpdateOverlay();
    void DestroyCommandBuffers();

    std::string GetWindowTitle() const;

public:
    bool prepared = false;
    bool resized = false;
    bool useGlslang = false;
    bool debugVulkan = true;
    bool useVsync = false;
    bool useFullScreen = false;
    bool requiresStencil = false;
    uint32_t width = 1280;
    uint32_t height = 720;

    std::string name = "VulkanExample";
    std::string title = "Demo";

    uint32_t apiVersion = VK_API_VERSION_1_0;
    VulkanDevice* vulkanDevice = {};
    Render::Vulkan::UIOverlay ui;

    uint32_t destWidth{};
    uint32_t destHeight{};

#if defined (_WIN32)
    HWND window;
    HINSTANCE windowInstance;
#endif

    uint32_t frameCounter = 0;

    void* m_deviceCreatepNextChain = nullptr;

public:
    bool InitVulkan();

    VkResult CreateInstance();

    virtual void Prepare();

    virtual void Render() = 0;

    void RenderLoop();

    void WindowResize();

    void DrawUI(const VkCommandBuffer commandBuffer);

    virtual void GetEnabledFeatures();

    virtual void GetEnabledExtensions();

public:

    ApplicationBase();
    virtual ~ApplicationBase();

#if defined( _WIN32 )
    void SetupConsole(std::string title);
    HWND SetUpWindow(HINSTANCE, WNDPROC);
    virtual void HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

};