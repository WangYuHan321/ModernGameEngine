cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(draw_triangle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_path(VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h
        HINTS ENV VULKAN_SDK
        PATH_SUFFIXES Include
)

set(Vulkan_MoltenVK_INCLUDE_DIR /Users/admin/VulkanSDK/1.4.321.0/iOS/include)
set(MoltenVK_LIB /Users/admin/VulkanSDK/1.4.321.0/iOS/lib)
set(MoltenVK_SDK /Users/admin/VulkanSDK/1.4.321.0)

find_library(QUARTZCORE_LIBRARY NAMES QuartzCore )
find_library(Vulkan_MoltenVK_LIBRARY NAMES Metal )
find_library(CoreGraphics NAMES CoreGraphics )
find_library(Foundation NAMES Foundation )
find_library(GLKit NAMES GLKit )
find_library(MoltenVK NAMES MoltenVK )
find_library(CoreAudio NAMES CoreAudio )
message(${Vulkan_INCLUDE_DIR}/..//MoltenVK/lib)
find_library(MOLTENVK_LIB PATHS  $ENV{VULKAN_SDK}/MoltenVK/lib)


file(GLOB IOS_SRC ./ios/* )
file(GLOB RES_SRC ./Resources/* )

SOURCE_GROUP(IOS FILES  ${IOS_SRC})
SOURCE_GROUP(Resources FILES  ${IOS_SRC})

include_directories( BEFORE
	${MoltenVK_SDK}
	${CMAKE_CURRENT_SOURCE_DIR}
    	${Vulkan_MoltenVK_INCLUDE_DIR}
)

add_executable(draw_triangle MVKExample.h MVKExample.cpp ${IOS_SRC} ${RES_SRC})

target_link_libraries(draw_triangle PRIVATE 
${QUARTZCORE_LIBRARY}
"-framework QuartzCore"
${Vulkan_MoltenVK_LIBRARY}
"-framework Metal"
${CoreGraphics}
"-framework CoreGraphics"
${Foundation}
"-framework Foundation"
${MoltenVK_LIB}
#"-framework MoltenVK"
${GLKit}
"-framework GLKit"
${CoreAudio}
"-framework CoreAudio"
)

