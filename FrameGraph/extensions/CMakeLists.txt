# =========================================================
#  FrameGraph Extensions
# =========================================================
#  这里集中登记随仓库附带的扩展 / 第三方库。
#  目前包含：
#    - vulkan_loader : Vulkan 动态函数加载器 + 各版本 Vulkan 头文件
#
#  说明：vulkan_loader 为 azhirnov/FrameGraph 原始源码，依赖其原始 STL
#  （FGC 命名空间、NtStringView / Singleton / StringUtils 等），目前与本项目的
#  STL/ 不兼容，因此由 FG_ENABLE_VULKAN_LOADER 控制是否参与构建（默认关闭）。
# =========================================================

if (FG_ENABLE_VULKAN_LOADER AND NOT (FG_VULKAN_VERSION STREQUAL "0"))
	message(STATUS "FrameGraph extensions: VulkanLoader enabled (Vulkan ${FG_VULKAN_VERSION})")
	add_subdirectory(vulkan_loader)
else ()
	message(STATUS "FrameGraph extensions: VulkanLoader disabled (set FG_ENABLE_VULKAN_LOADER=ON to build)")
endif ()
