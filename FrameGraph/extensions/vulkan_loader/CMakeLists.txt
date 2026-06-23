if (NOT (${FG_VULKAN_VERSION} STREQUAL "0"))
	file(GLOB_RECURSE SOURCES
		"*.cpp"
		"*.h"
		"vk_loader/*"
	)
	# 排除已废弃的小写 stl/ 目录（统一使用 ../../STL）
	list(FILTER SOURCES EXCLUDE REGEX "/stl/")

	add_library("VulkanLoader" STATIC ${SOURCES})
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
	target_include_directories("VulkanLoader" PUBLIC "..")
	set_property(TARGET "VulkanLoader" PROPERTY FOLDER "Extensions")

	get_filename_component(_FG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
	target_include_directories("VulkanLoader" PUBLIC "${_FG_ROOT}/STL")

	if (${FG_VULKAN_VERSION} STREQUAL "100")
		target_include_directories("VulkanLoader" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/1.0")
	elseif (${FG_VULKAN_VERSION} STREQUAL "110")
		target_include_directories("VulkanLoader" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/1.1")
	elseif (${FG_VULKAN_VERSION} STREQUAL "120")
		target_include_directories("VulkanLoader" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/1.2")
	else()
		message(FATAL_ERROR "unknown Vulkan version: ${FG_VULKAN_VERSION}")
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		target_compile_definitions("VulkanLoader" PUBLIC VK_USE_PLATFORM_ANDROID_KHR=1)
	endif()

	target_compile_definitions("VulkanLoader" PUBLIC FG_ENABLE_VULKAN)
	target_compile_definitions("VulkanLoader" PUBLIC FG_VULKAN_TARGET_VERSION=${FG_VULKAN_VERSION})

	install(TARGETS "VulkanLoader" ARCHIVE DESTINATION "libs/$<CONFIG>")
	install(DIRECTORY "." DESTINATION "include/VulkanLoader" FILES_MATCHING PATTERN "*.h*")
endif()
