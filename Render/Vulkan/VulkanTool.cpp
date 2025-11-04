#include "VulkanTool.h"
#include "VulkanInitializers.hpp"

void Render::Vulkan::Tool::SetImageLayout(
        VkCommandBuffer commandBuffer,
        VkImage         image,
        VkImageLayout   oldImageLayout,
        VkImageLayout   newImageLayout,
        VkImageSubresourceRange subresourceRange,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask)
{
    /*oldImageLayout：图片当前的布局。
    newImageLayout：你想要切换到的布局。
    srcAccessMask：在切换前需要完成 / 可见的访问类型，确保在进入新布局前这些操作已经完成、对后续访问可见。
    dstAccessMask：切换到新布局后，目标访问类型需要的内存 / 执行屏障。*/

    VkImageMemoryBarrier imageMemoryBarrier = Render::Vulkan::Initializer::ImageMemoryBarrier();
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    switch (oldImageLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED://图片当前无定义布局，常用于初始状态；不保留数据。
            srcStageMask = 0;//没有已发生的访问需要等待。
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED://图片已被主机/CPU 预初始化写入过（线性图片较常见）。
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;//保证主机端写入完成后，设备再访问图片。
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: //图片将作为颜色附件使用（渲染目标）。
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//确保在进入新布局前，颜色附件写入已经完成。
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL://图片将作为深度/模板（深度/模板缓冲区）使用。
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;//确保在进入新布局前，传输读取/相关操作完成。
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL://图片将作为传输源使用（读数据）。
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;//确保在进入新布局前，传输读取/相关操作完成。
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL://图片将作为传输目标使用（写数据）。
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;//确保在进入新布局前，传输写入完成。
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL://图片将被着色器只读访问（如采样器）。
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;//确保之前的写入对着色器可见。
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
    }

    switch (newImageLayout)
    {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL://图片将作为传输目标使用（写数据）。
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;//目标是写入到图片，通过传输进行。。
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;//目标是从图片读取，通过传输进行。。
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//目标是作为颜色附件写入。
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL://目标是作为深度/模板附件写入；注意用 OR 以免覆盖原有值。
            imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if (imageMemoryBarrier.srcAccessMask == 0)//图片将被着色器只读访问（如采样器）。
            {
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;//之前没有显式指定过任何访问（例如从 UNDEFINED 转到只读），就假设可能有主机写入或传输写入需要同步，确保这些写入对着色器读取可见。
            }
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
}

void Render::Vulkan::Tool::SetImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage         image,
    VkImageAspectFlags aspectMask,
    VkImageLayout   oldImageLayout,
    VkImageLayout   newImageLayout,
    VkPipelineStageFlags srcStageMask, //Ĭ�� ������ָ��׶ζ�Ҫ������
    VkPipelineStageFlags dstStageMask)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    SetImageLayout(commandBuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

VkCommandBuffer BeginSingleTimeCommands(Render::Vulkan::VulkanDevice* device, VkCommandPool cmdPool)
{
    // 和渲染一样，使用CommandBuffer拷贝缓存
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void EndSingleTimeCommands(Render::Vulkan::VulkanDevice* device, VkCommandPool cmdPool, VkCommandBuffer commandBuffer,VkQueue queue)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device->logicalDevice, cmdPool, 1, &commandBuffer);
}

void Render::Vulkan::Tool::GenerateMipMap(Render::Vulkan::VulkanDevice* device,
                                          VkQueue queue, VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    // 检查图像格式是否支持 linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, device->commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    EndSingleTimeCommands(device, device->commandPool,commandBuffer, queue);
}

VkBool32 Render::Vulkan::Tool::GetSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat)
{
    std::vector<VkFormat> formatList = {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (auto& format : formatList)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depthStencilFormat = format;
            return true;
        }
    }

    return false;
}

VkBool32 Render::Vulkan::Tool::GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat* depthFormat)
{
    std::vector<VkFormat> formatList = {
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM
    };

    for (auto& format : formatList)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *depthFormat = format;
            return true;
        }
    }

    return false;
}

#if defined (__ANDROID__)

VkShaderModule Render::Vulkan::Tool::LoadShader(AAssetManager* assetManager, const char *fileName, VkDevice device)
{
    // Load shader from compressed asset
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_STREAMING);
    assert(asset);
    size_t size = AAsset_getLength(asset);
    assert(size > 0);

    char *shaderCode = new char[size];
    AAsset_read(asset, shaderCode, size);
    AAsset_close(asset);

    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = (uint32_t*)shaderCode;
    moduleCreateInfo.flags = 0;

    VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

    delete[] shaderCode;

    return shaderModule;
}

#else

VkShaderModule Render::Vulkan::Tool::LoadShader(const char* fileName, VkDevice device)
{
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open())
    {
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        char* shaderCode = new char[size];
        is.read(shaderCode, size);
        is.close();

        assert(size > 0);

        VkShaderModule shaderModule;
        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = size;
        moduleCreateInfo.pCode = (uint32_t*)shaderCode;

        VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

        delete[] shaderCode;

        return shaderModule;
    }
    else
    {
        std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << "\n";
        return VK_NULL_HANDLE;
    }
}

#endif

std::string Render::Vulkan::Tool::ErrorString(VkResult errorCode)
{
    switch (errorCode)
    {
#define STR(r) case VK_ ##r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
        STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef STR
    default:
        return "UNKNOWN_ERROR";
    }
}

