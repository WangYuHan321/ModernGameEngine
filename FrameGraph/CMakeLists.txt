
#file(GLOB BASE_SRC "./include/*.h" "./src/*.pp")
file(GLOB PUBLIC_SRC "./Public/*.h" "./Public/*.cpp" "./Public/*.hpp")
file(GLOB SHARED_SRC "./Shared/*.h" "./Shared/*.cpp" "./Shared/*.hpp")
file(GLOB VULKAN_SRC "./Vulkan/*.h" "./Vulkan/*.cpp" "./Vulkan/*.hpp")
file(GLOB STL_SRC "./STL/*.h" "./STL/*.cpp" "./STL/*.hpp")
file(GLOB STL_ALGORITHMS_SRC "./STL/Algorithms/*.h" "./STL/Algorithms/*.cpp" "./STL/Algorithms/*.hpp")
file(GLOB STL_CONTAINERS_SRC "./STL/Containers/*.h" "./STL/Containers/*.cpp" "./STL/Containers/*.hpp")
file(GLOB STL_MEMORY_SRC "./STL/Memory/*.h" "./STL/Memory/*.cpp" "./STL/Memory/*.hpp")
file(GLOB STL_MATH_SRC "./STL/Math/*.h" "./STL/Math/*.cpp" "./STL/Math/*.hpp")
file(GLOB STL_COMPILETIME_SRC "./STL/CompileTime/*.h" "./STL/CompileTime/*.cpp" "./STL/CompileTime/*.hpp")

SOURCE_GROUP(Public FILES  ${PUBLIC_SRC})
SOURCE_GROUP(Shared FILES  ${SHARED_SRC})
SOURCE_GROUP(Vulkan FILES  ${VULKAN_SRC})
SOURCE_GROUP(STL FILES  ${STL_SRC})
SOURCE_GROUP(STL/Algorithms FILES  ${STL_ALGORITHMS_SRC})
SOURCE_GROUP(STL/Containers FILES  ${STL_CONTAINERS_SRC})
SOURCE_GROUP(STL/Memory FILES  ${STL_MEMORY_SRC})
SOURCE_GROUP(STL/Math FILES  ${STL_MATH_SRC})
SOURCE_GROUP(STL/CompileTime FILES  ${STL_COMPILETIME_SRC})



project(FrameGraph)

if (MSVC)
    add_compile_options("/utf-8")
endif()

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
	${STL_SRC}
	${STL_MATH_SRC}
	${STL_MEMORY_SRC}
	${STL_CONTAINERS_SRC}
	${STL_ALGORITHMS_SRC}
	${STL_COMPILETIME_SRC}
	${IMGUI_SRC}
	)

include_directories( BEFORE
	${DEP_DIR}
	${IMGUI_DIR}
	${STB_INCLUDE_DIR}
	${CMAKE_CURRENMT}
	${GLM_INCLUDE_DIR}
	${Vulkan_INCLUDE_DIR}
	${GLSLANG_INCLUDE_DIR}
	${TINYGLTF_INCLUDE_DIR}
	${GLFW_BINARY_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(FrameGraph STATIC ${SRC_FILES})

target_link_libraries(FrameGraph PRIVATE  ${DEPENDENCY_FILES})



