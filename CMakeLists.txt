cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(DEP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps) 
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm)
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb)  
set(TINYGLTF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/tinygltf)  

if(WIN32)
    message(STATUS "current platform is Window Os !!!")
elseif(UNIX AND NOT APPLE)
    message(STATUS "current platform is Linux Os !!!")
elseif(APPLE)
    message(STATUS "current platform is Mac Os !!!")
else()

endif(WIN32)

include(cmake/FindVulkan.cmake)

add_subdirectory(deps/glm)
add_subdirectory(deps/glfw)
#add_subdirectory(deps/stb)
add_subdirectory(deps/glslang)
add_subdirectory(deps/tinygltf)
#add_subdirectory(deps/taskflow)

add_subdirectory(Unit_Test/draw_mesh)
add_subdirectory(Unit_Test/draw_texture)
add_subdirectory(Unit_Test/draw_triangle)



