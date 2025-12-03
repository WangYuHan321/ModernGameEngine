#pragma once

#include "VulkanDevice.h"
#include "VulkanTool.h"
#include "VulkanInitializers.hpp"
#include "imgui/imgui.h"
#include <glm/glm.hpp>

namespace Render
{
    namespace Vulkan 
    {
        struct PushConstBlock
        {
            glm::vec2 scale;
            glm::vec2 translate;
        };

        class UIOverlay
        {
        public:
            VulkanDevice* device{ nullptr };
            VkQueue queue{ VK_NULL_HANDLE };

            VkSampleCountFlagBits rasterizationSamples{ VK_SAMPLE_COUNT_1_BIT };
            uint32_t subPass{ 0 };

            Buffer vertexBuffer;
            Buffer indexBuffer;
            int32_t vertexCount{ 0 };
            int32_t indexCount{ 0 };

            std::vector<VkPipelineShaderStageCreateInfo> shaders;

            VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
            VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
            VkDescriptorSet descriptorSet;
            VkPipeline pipeline{ VK_NULL_HANDLE };
            VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

            VkDeviceMemory fontMemory{ VK_NULL_HANDLE };
            VkImage fontImage{ VK_NULL_HANDLE };
            VkImageView fontView{ VK_NULL_HANDLE };
            VkSampler sampler{ VK_NULL_HANDLE };

            PushConstBlock pushConstBlock;

            bool visible{ true };
            float scale{ 1.0f };

            UIOverlay();
            ~UIOverlay();

            void PrepareResource();
            void PreparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat);

            bool Update();
            void Draw(const VkCommandBuffer cmdBuf);
            void Resize(uint32_t width, uint32_t height);

            void FreeResource();

            bool Header(const char* caption);

            void Text(const char* pStr, ...);

            bool CheckBox(const char* caption, bool* value);
        };
    }
}