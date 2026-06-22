cmake_minimum_required(VERSION 3.16)

#file(GLOB BASE_SRC "./include/*.h" "./src/*.pp")
file(GLOB PUBLIC_SRC "./Public/*.h" "./Public/*.cpp" "./Public/*.hpp")
file(GLOB SHARED_SRC "./Shared/*.h" "./Shared/*.cpp" "./Shared/*.hpp")
file(GLOB VULKAN_SRC "./Vulkan/*.h" "./Vulkan/*.cpp" "./Vulkan/*.hpp")
file(GLOB VULKAN_BUFFER_SRC "./Vulkan/Buffer/*.h" "./Vulkan/Buffer/*.cpp" "./Vulkan/Buffer/*.hpp")
file(GLOB VULKAN_MEMORY_SRC "./Vulkan/Memory/*.h" "./Vulkan/Memory/*.cpp" "./Vulkan/Memory/*.hpp")
file(GLOB VULKAN_UTILS_SRC "./Vulkan/Utils/*.h" "./Vulkan/Utils/*.cpp" "./Vulkan/Utils/*.hpp")
file(GLOB VULKAN_INSTANCE_SRC "./Vulkan/Instance/*.h" "./Vulkan/Instance/*.cpp" "./Vulkan/Instance/*.hpp")
file(GLOB STL_SRC "./STL/*.h" "./STL/*.cpp" "./STL/*.hpp")
file(GLOB STL_ALGORITHMS_SRC "./STL/Algorithms/*.h" "./STL/Algorithms/*.cpp" "./STL/Algorithms/*.hpp")
file(GLOB STL_CONTAINERS_SRC "./STL/Containers/*.h" "./STL/Containers/*.cpp" "./STL/Containers/*.hpp")
file(GLOB STL_MEMORY_SRC "./STL/Memory/*.h" "./STL/Memory/*.cpp" "./STL/Memory/*.hpp")
file(GLOB STL_LOG_SRC "./STL/Log/*.h" "./STL/Log/*.cpp" "./STL/Log/*.hpp")
file(GLOB STL_MATH_SRC "./STL/Math/*.h" "./STL/Math/*.cpp" "./STL/Math/*.hpp")
file(GLOB STL_THREADSAFE_SRC "./STL/ThreadSafe/*.h" "./STL/ThreadSafe/*.cpp" "./STL/ThreadSafe/*.hpp")
file(GLOB STL_COMPILETIME_SRC "./STL/CompileTime/*.h" "./STL/CompileTime/*.cpp" "./STL/CompileTime/*.hpp")

SOURCE_GROUP(Public FILES  ${PUBLIC_SRC})
SOURCE_GROUP(Shared FILES  ${SHARED_SRC})
SOURCE_GROUP(Vulkan FILES  ${VULKAN_SRC})
SOURCE_GROUP(Vulkan/Buffer FILES  ${VULKAN_BUFFER_SRC})
SOURCE_GROUP(Vulkan/Memory FILES  ${VULKAN_MEMORY_SRC})
SOURCE_GROUP(Vulkan/Utils FILES  ${VULKAN_UTILS_SRC})
SOURCE_GROUP(Vulkan/Instance FILES  ${VULKAN_INSTANCE_SRC})
SOURCE_GROUP(STL FILES  ${STL_SRC})
SOURCE_GROUP(STL/Algorithms FILES  ${STL_ALGORITHMS_SRC})
SOURCE_GROUP(STL/Containers FILES  ${STL_CONTAINERS_SRC})
SOURCE_GROUP(STL/Memory FILES  ${STL_MEMORY_SRC})
SOURCE_GROUP(STL/Log FILES  ${STL_LOG_SRC})
SOURCE_GROUP(STL/Math FILES  ${STL_MATH_SRC})
SOURCE_GROUP(STL/ThreadSafe FILES  ${STL_THREADSAFE_SRC})
SOURCE_GROUP(STL/CompileTime FILES  ${STL_COMPILETIME_SRC})



project(FrameGraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options("/utf-8" "/FS")
endif()

# =========================================================
#  Extensions（扩展 / 第三方库）
# =========================================================
# 目标 Vulkan 版本：100 / 110 / 120（0 = 关闭扩展中的 Vulkan 相关库）
if (NOT DEFINED FG_VULKAN_VERSION)
	set(FG_VULKAN_VERSION "120" CACHE STRING "Target Vulkan version for extensions: 100 / 110 / 120 (0 = off)")
endif()

# 是否构建随仓库附带的 VulkanLoader 扩展库。
# 该扩展为 azhirnov/FrameGraph 的原始源码，依赖其原始 STL（FGC 命名空间、
# NtStringView / Singleton / StringUtils / BEGIN_ENUM_CHECKS 等），与本项目的
# STL/ 尚不兼容；且本项目当前直接使用 vulkan.h 原型函数，无需该动态加载器即可工作，
# 故默认关闭。需要时打开（并先把其源码移植到本项目 STL）。
option(FG_ENABLE_VULKAN_LOADER "Build the bundled VulkanLoader extension (needs source port to this project's STL)" OFF)

# 把 extensions 目录纳入构建树（其中按需登记各扩展 / 第三方库）
add_subdirectory(extensions)

# ---------------------------------------------------------
#  Vulkan 头文件来源
# ---------------------------------------------------------
# 使用扩展里自带的 Vulkan 头文件（省去对系统 Vulkan SDK 头的依赖）。
# 注意：链接阶段仍需系统 Vulkan 运行库（${Vulkan_LIBRARY}），因为本项目
# 直接使用 vulkan.h 的原型函数。
option(FG_USE_BUNDLED_VULKAN_HEADERS "Use Vulkan headers bundled in extensions/vulkan_loader instead of the system SDK" ON)

if (FG_VULKAN_VERSION STREQUAL "100")
	set(FG_BUNDLED_VULKAN_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extensions/vulkan_loader/1.0")
elseif (FG_VULKAN_VERSION STREQUAL "110")
	set(FG_BUNDLED_VULKAN_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extensions/vulkan_loader/1.1")
elseif (FG_VULKAN_VERSION STREQUAL "120")
	set(FG_BUNDLED_VULKAN_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extensions/vulkan_loader/1.2")
endif()

if (FG_USE_BUNDLED_VULKAN_HEADERS AND DEFINED FG_BUNDLED_VULKAN_HEADERS_DIR)
	set(FG_VULKAN_INCLUDE_DIR "${FG_BUNDLED_VULKAN_HEADERS_DIR}")
	message(STATUS "FrameGraph: using bundled Vulkan headers -> ${FG_VULKAN_INCLUDE_DIR}")
else ()
	set(FG_VULKAN_INCLUDE_DIR "${Vulkan_INCLUDE_DIR}")
	message(STATUS "FrameGraph: using system Vulkan headers -> ${FG_VULKAN_INCLUDE_DIR}")
endif ()

if(WIN32)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY})	
elseif(APPLE)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
else()
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY})
endif(WIN32)


set (IMGUI_SRC
	${IMGUI_DIR}/imconfig.h
	${IMGUI_DIR}/imgui.h
	${IMGUI_DIR}/imgui_internal.h
	${IMGUI_DIR}/imstb_rectpack.h
	${IMGUI_DIR}/imstb_textedit.h
	${IMGUI_DIR}/imstb_truetype.h
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp	
)

set(SRC_FILES 
	${PUBLIC_SRC}
	${SHARED_SRC}
	${VULKAN_SRC}
	${VULKAN_BUFFER_SRC}
	${VULKAN_MEMORY_SRC}
	${VULKAN_UTILS_SRC}
	${VULKAN_INSTANCE_SRC}
	${STL_SRC}
	${STL_LOG_SRC}
	${STL_MATH_SRC}
	${STL_MEMORY_SRC}
	${STL_THREADSAFE_SRC}
	${STL_CONTAINERS_SRC}
	${STL_ALGORITHMS_SRC}
	${STL_COMPILETIME_SRC}
	${IMGUI_SRC}
	)

include_directories( BEFORE
	${FG_VULKAN_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/extensions
	${CMAKE_CURRENT_SOURCE_DIR}/STL
	${DEP_DIR}
	${IMGUI_DIR}
	${STB_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Vulkan
	${GLM_INCLUDE_DIR}
	${GLSLANG_INCLUDE_DIR}
	${TINYGLTF_INCLUDE_DIR}
	${GLFW_BINARY_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(FrameGraph STATIC ${SRC_FILES})

# 静态库直接输出到项目根目录（FrameGraph.lib）
set(_FG_LIB_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(FrameGraph PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${_FG_LIB_OUT_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${_FG_LIB_OUT_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${_FG_LIB_OUT_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_FG_LIB_OUT_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${_FG_LIB_OUT_DIR}"
)

target_link_libraries(FrameGraph PRIVATE  ${DEPENDENCY_FILES})

# 链接扩展库（仅当对应目标被成功创建时）
if (TARGET VulkanLoader)
	target_link_libraries(FrameGraph PUBLIC VulkanLoader)
endif()



