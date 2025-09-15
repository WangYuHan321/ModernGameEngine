
file(GLOB INCLUDE_FILES include/*.hpp)
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB INCLUDE_HFILES include/*.h)


project(draw_triangle)

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

add_executable(draw_triangle ${SRC_FILES})

if(WIN32)
	target_link_libraries(draw_triangle PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(draw_triangle PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)





