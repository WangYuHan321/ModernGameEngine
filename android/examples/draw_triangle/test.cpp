#include <android_native_app_glue.h>
#include "VulkanAndroid.h"

void android_main(android_app* state)																
{
   androidApp = state;
   Render::Vulkan::android::GetDeviceConfig();
    __android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Loading libVulkan.so...\n");
}