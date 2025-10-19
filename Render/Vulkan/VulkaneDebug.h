#pragma once

#if defined(__ANDROID__)
    #include "VulkanAndroid.h"
#else
    #include <vulkan/vulkan.h>
#endif

#include "VulkanTool.h"
#include <glm/glm.hpp>

namespace Render
{
    namespace Vulkan 
    {
        namespace Debug 
        {
            extern bool mblogToFile;
            extern std::string mlogFileName;

            VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessageCallback(
                    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                    void *pUserData);

            void SetupDebugging(VkInstance instance);

            void FreeDebugCallback(VkInstance instance);

            void SetupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugUtilsMessengerCI);

            void Log(std::string message);
        }

        // Wrapper for the VK_EXT_debug_utils extension
        // These can be used to name Vulkan objects for debugging tools like RenderDoc
        namespace DebugUtils {
            void Setup(VkInstance instance);

            void CmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, glm::vec4 color);

            void CmdEndLabel(VkCommandBuffer cmdbuffer);
        }
    }
}
