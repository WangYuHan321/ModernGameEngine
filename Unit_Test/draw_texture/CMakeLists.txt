
file(GLOB INCLUDE_FILES include/*.hpp)
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB INCLUDE_HFILES include/*.h)


project(draw_texture)

if(WIN32)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} glfw)	
elseif(APPLE)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} glfw)
endif(WIN32)

			

set(SRC_FILES 
	${INCLUDE_FILES}
	${SOURCE_FILES}
	${INCLUDE_HFILES}
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

add_executable(draw_texture ${SRC_FILES})

if(WIN32)
	target_link_libraries(draw_texture PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(draw_texture PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)

set_target_properties(draw_texture PROPERTIES CMAKE_WORKING_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

set_target_properties(draw_texture PROPERTIES RUNTIME_WORKING_DIRECTORY_DEBUG "${EXECUTABLE_OUTPUT_PATH}")
set_target_properties(draw_texture PROPERTIES RUNTIME_WORKING_DIRECTORY_RELEASE "${EXECUTABLE_OUTPUT_PATH}")
set_target_properties(draw_texture PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${EXECUTABLE_OUTPUT_PATH}")
set_target_properties(draw_texture PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${EXECUTABLE_OUTPUT_PATH}")
set_target_properties(draw_texture PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")




