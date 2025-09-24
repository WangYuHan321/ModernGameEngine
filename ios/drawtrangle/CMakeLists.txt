cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(draw_triangle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

  find_library(QUARTZCORE_LIBRARY
    NAMES QuartzCore
  )
find_path(MOLTENVK_INCLUDE_DIR vulkan/vulkan.h HINTS "/usr/local/include" "/usr/local/opt/molten-vk/include")
find_library(MOLTENVK_LIB NAMES MoltenVK PATHS "/usr/local/lib" "/usr/local/opt/molten-vk/lib")

file(GLOB IOS_SRC ./ios/* )
file(GLOB RES_SRC ./Resources/* )

SOURCE_GROUP(IOS FILES  ${IOS_SRC})
SOURCE_GROUP(Resources FILES  ${IOS_SRC})

include_directories( BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(draw_triangle MVKExample.h MVKExample.cpp ${IOS_SRC} ${RES_SRC})

target_link_libraries(draw_triangle PRIVATE ${QUARTZCORE_LIBRARY}
"-framework QuartzCore" 
                        "-framework Metal" 
                        "-framework Foundation" 
                        "-framework IOSurface" 
                        "-framework CoreGraphics" 
                        "-framework IOKit" 
                        "-framework AppKit"
${MOLTENVK_LIB})

