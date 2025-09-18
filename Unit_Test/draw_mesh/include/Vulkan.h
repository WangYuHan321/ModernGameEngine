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

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};


const int MAX_FRAMES_IN_FLIGHT = 2;

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

    void UpdateUniformBuffer(uint32_t currentImage);

    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
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
