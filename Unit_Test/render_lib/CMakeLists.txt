
#file(GLOB BASE_SRC "./include/*.h" "./src/*.pp")
file(GLOB RENDER_SRC "../../Render/*.h" "../../Render/*.cpp" "../../Render/*.hpp")
file(GLOB VULKAN_SRC "../../Render/Vulkan/*.h" "../../Render/Vulkan/*.cpp" "../../Render/Vulkan/*.hpp")

SOURCE_GROUP(Render FILES  ${RENDER_SRC})
SOURCE_GROUP(Render/Vulkan FILES  ${VULKAN_SRC})

project(RenderLib)

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
	#${BASE_SRC}
	${RENDER_SRC}
	${VULKAN_SRC}
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

add_library(RenderLib STATIC ${SRC_FILES})

target_link_libraries(RenderLib PRIVATE  ${DEPENDENCY_FILES})



