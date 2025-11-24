#pragma once
#include <vector>

#if defined (WIN32)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#ifdef __APPLE__
#include <sys/utsname.h>
#endif

#include <vulkan/vulkan.h>
#include "VulkanTool.h"

namespace Render
{
	namespace Vulkan
	{
		class VulkanSwapChain
		{
		private:
			VkInstance instance{ VK_NULL_HANDLE };
			VkDevice device{ VK_NULL_HANDLE };
			VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
			VkSurfaceKHR surface{ VK_NULL_HANDLE };

		public:
			VkFormat colorFormat{};
			VkColorSpaceKHR colorSpace{};
			VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
			std::vector<VkImage> images{};
			std::vector<VkImageView> imageViews{};
			uint32_t queueNodeIndex{ UINT32_MAX };

#if defined (VK_USE_PLATFORM_WIN32_KHR)//VK_USE_PLATFORM_WIN32_KHR
			void InitSurface(void* platformHandle, void* platformWindow);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            void InitSurface(ANativeWindow* window);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
            void InitSurface(CAMetalLayer* metalLayer);
#endif

			void SetContext(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

			void Create(uint32_t width, uint32_t height, bool vsync = false, bool fullScreen = false);
	
			VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);

			VkResult QueuePresent(VkQueue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

			void CleanUp();
		};
	}
}
