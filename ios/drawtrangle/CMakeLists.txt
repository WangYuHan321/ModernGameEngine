cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(draw_triangle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_library(QUARTZCORE_LIBRARY NAMES QuartzCore )
find_library(Vulkan_MoltenVK_LIBRARY NAMES Metal )
find_library(CoreGraphics NAMES CoreGraphics )
find_library(Foundation NAMES Foundation )
find_library(GLKit NAMES GLKit )
find_library(MoltenVK NAMES MoltenVK )
find_library(CoreAudio NAMES CoreAudio )



file(GLOB IOS_SRC ./ios/* )
file(GLOB RES_SRC ./Resources/* )

SOURCE_GROUP(IOS FILES  ${IOS_SRC})
SOURCE_GROUP(Resources FILES  ${IOS_SRC})

include_directories( BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}
    	#${Vulkan_MoltenVK_INCLUDE_DIR}
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
${GLKit}
"-framework GLKit"
${MoltenVK}
${CoreAudio}
"-framework CoreAudio"
)

