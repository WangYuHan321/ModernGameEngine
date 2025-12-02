cmake_minimum_required (VERSION 3.5)
cmake_policy(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(DEP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps) 
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm)
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb) 
set(RENDER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Render) 
set(TINYGLTF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/tinygltf)
set(GLSLANG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glslang/glslang/include)    

if(WIN32)
    message(STATUS "current platform is Window Os !!!")
elseif(LINUX)
    message(STATUS "current platform is Linux Os !!!")
elseif(APPLE)
    message(STATUS "current platform is Apple !!!")
    set(CMAKE_SUPPRESS_REGENERATION ON)
    set(CMAKE_XCODE_GENERATE_SCHEME ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_METAL_EXT -DVK_EXAMPLE_XCODE_GENERATED")
    link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")

    if(CMAKE_GENERATOR MATCHES "Makefiles" OR CMAKE_GENERATOR MATCHES "Ninja")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -ObjC++")
	else() # SRS - use objective-c++ language flag vs. ObjC++ for Xcode builds
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -xobjective-c++")
	endif()
endif(WIN32)

include(cmake/FindVulkan.cmake)

add_subdirectory(deps/glm)
add_subdirectory(deps/glfw)
#add_subdirectory(deps/stb)
add_subdirectory(deps/glslang)
add_subdirectory(deps/tinygltf)
#add_subdirectory(deps/taskflow)

add_subdirectory(Unit_Test/render_lib)
add_subdirectory(Unit_Test/draw_mesh)
add_subdirectory(Unit_Test/draw_model)
add_subdirectory(Unit_Test/draw_compute)
add_subdirectory(Unit_Test/draw_texture)
add_subdirectory(Unit_Test/draw_triangle)
add_subdirectory(Unit_Test/ResourceManager)

if(WIN32)
	add_subdirectory(Unit_Test/dx12_triangle)
endif(WIN32)



