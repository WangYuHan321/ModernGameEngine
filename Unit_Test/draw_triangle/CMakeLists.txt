
file(GLOB INCLUDE_FILES include/*.hpp)
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB INCLUDE_HFILES include/*.h)

if (MSVC)
    add_compile_options("/utf-8")
endif()


project(draw_triangle)

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
	main.cpp
	)

include_directories( BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${STB_INCLUDE_DIR}
	${Vulkan_INCLUDE_DIR}
	${CMAKE_CURRENMT}
	${GLFW_BINARY_DIR}/include
)

link_directories(
  ${VK_SDK_PATH}/Lib
)

add_executable(draw_triangle ${SRC_FILES})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")

if(WIN32)
	target_link_libraries(draw_triangle PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(draw_triangle PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)





