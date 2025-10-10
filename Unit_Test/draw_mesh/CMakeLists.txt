
file(GLOB INCLUDE_FILES include/*.hpp)
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB INCLUDE_HFILES include/*.h)


project(draw_mesh)

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
	${INCLUDE_FILES}
	${SOURCE_FILES}
	${INCLUDE_HFILES}
	${IMGUI_SRC}
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
	main.cpp
	)

include_directories( BEFORE
	${DEP_DIR}
	${IMGUI_DIR}
	${STB_INCLUDE_DIR}
	${CMAKE_CURRENMT}
	${GLM_INCLUDE_DIR}
	${Vulkan_INCLUDE_DIR}
	${TINYGLTF_INCLUDE_DIR}
	${GLFW_BINARY_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
  ${VK_SDK_PATH}/Lib
)

add_executable(draw_mesh ${SRC_FILES})

if(WIN32)
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif(WIN32)
	
if(APPLE)
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
endif(APPLE)


if(WIN32)
	target_link_libraries(draw_mesh PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(draw_mesh PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)




