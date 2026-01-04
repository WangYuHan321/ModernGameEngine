
file(GLOB BASE_SRC "./include/*.h" "./include/*.hpp" "./src/*.cpp")

set(MAIN_DIR  "../../")
set(RENDER_INCLUDE_DIR ${MAIN_DIR}/Render)

SOURCE_GROUP(Render FILES  ${RENDER_SRC})
SOURCE_GROUP(Render/Vulkan FILES  ${VULKAN_SRC})

project(draw_multhread)

if (MSVC)
    add_compile_options("/utf-8")
endif()

if(WIN32)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} RenderLib)	
elseif(APPLE)
	set(DEPENDENCY_FILES ${Vulkan_LIBRARY} RenderLib "-framework AppKit" "-framework QuartzCore")
endif(WIN32)

set(SRC_FILES 
	${BASE_SRC}
	${VULKAN_SRC}
	main.cpp
	)

include_directories( BEFORE
	${DEP_DIR}
	${IMGUI_DIR}
	${MAIN_DIR}
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

link_directories( ${VK_SDK_PATH}/Lib)

if(WIN32)
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
endif(WIN32)
	
if(APPLE)
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../Asset" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release")
endif(APPLE)

add_executable(draw_multhread ${SRC_FILES})

if(WIN32)
	target_link_libraries(draw_multhread PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(draw_multhread PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)




