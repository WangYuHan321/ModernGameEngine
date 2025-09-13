cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(WIN32)
    message(STATUS "current platform is Window Os !!!")
elseif(UNIX AND NOT APPLE)
    message(STATUS "current platform is Linux Os !!!")
elseif(APPLE)
    message(STATUS "current platform is Mac Os !!!")
else()

endif(WIN32)

include(cmake/FindVulkan.cmake)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glslang)


add_subdirectory(Unit_Test/draw_triangle)



