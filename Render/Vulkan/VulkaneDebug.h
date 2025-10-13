#pragma once

#if defined(__ANDROID__)
    #include "VulkanAndroid.h"
#endif
#include <glm/glm.hpp>

namespace Render
{
    namespace Vulkan {
        namespace debug {
            extern bool logToFile;
            extern std::string logFileName;

            VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessageCallback(
                    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                    void *pUserData);

            void SetupDebugging(VkInstance instance);

            void FreeDebugCallback(VkInstance instance);

            void SetupDebugingMessengerCreateInfo(
                    VkDebugUtilsMessengerCreateInfoEXT &debugUtilsMessengerCI);

            void Log(std::string message);
        }

        // Wrapper for the VK_EXT_debug_utils extension
        // These can be used to name Vulkan objects for debugging tools like RenderDoc
        namespace debugutils {
            void Setup(VkInstance instance);

            void CmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, glm::vec4 color);

            void CmdEndLabel(VkCommandBuffer cmdbuffer);
        }
    }
}
