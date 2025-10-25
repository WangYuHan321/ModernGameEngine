
file(GLOB BASE_SRC "./include/*.h" "./include/*.hpp" "./src/*.cpp")
file(GLOB RENDER_SRC "../../Render/*.h" "../../Render/*.cpp" "../../Render/*.hpp")
file(GLOB VULKAN_SRC "../../Render/Vulkan/*.h" "../../Render/Vulkan/*.cpp" "../../Render/Vulkan/*.hpp")

set(RENDER_INCLUDE_DIR  "../../")

SOURCE_GROUP(Render FILES  ${RENDER_SRC})
SOURCE_GROUP(Render/Vulkan FILES  ${VULKAN_SRC})

project(draw_triangle)

if (MSVC)
    add_compile_options("/utf-8")
endif()

if(WIN32)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} glfw)	
elseif(APPLE)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} glfw)
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
	${BASE_SRC}
	${RENDER_SRC}
	${VULKAN_SRC}
	${IMGUI_SRC}
	main.cpp
	)

include_directories( BEFORE
	${DEP_DIR}
	${STB_INCLUDE_DIR}
	${CMAKE_CURRENMT}
	${GLM_INCLUDE_DIR}
	${RENDER_INCLUDE_DIR}
	${Vulkan_INCLUDE_DIR}
	${GLSLANG_INCLUDE_DIR}
	${TINYGLTF_INCLUDE_DIR}
	${GLFW_BINARY_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
  ${VK_SDK_PATH}/Lib
)

if(WIN32)
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif(WIN32)
	
if(APPLE)
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
endif(APPLE)

add_executable(draw_triangle ${SRC_FILES})

if(WIN32)
	target_link_libraries(draw_triangle PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(draw_triangle PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)




