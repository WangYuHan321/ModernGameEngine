set (IMPORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Import)


file(GLOB INCLUDE_FILES include/*.hpp)
file(GLOB SOURCE_FILES src/*.cpp  ${IMPORT_DIR}/Base/*.cpp)
file(GLOB INCLUDE_HFILES include/*.h)

file(GLOB IMPORT_SRC_FILE ${IMPORT_DIR}/*.cpp ${IMPORT_DIR}/*.cc ${IMPORT_DIR}/*.h)
file(GLOB IMPORT_SRC_CONFIG_FILE ${IMPORT_DIR}/Config/*.cpp ${IMPORT_DIR}/Config/*.h)
file(GLOB IMPORT_SRC_ASSET_FILE ${IMPORT_DIR}/Asset/*.cpp ${IMPORT_DIR}/Asset/*.h)
file(GLOB IMPORT_SRC_ASSET_BASE_FILE ${IMPORT_DIR}/Asset/Base/*.cpp ${IMPORT_DIR}/Asset/Base/*.h)

SOURCE_GROUP(Impore FILES ${IMPORT_SRC_FILE})
SOURCE_GROUP(Impore/Asset FILES ${IMPORT_SRC_ASSET_FILE})
SOURCE_GROUP(Impore/Config FILES ${IMPORT_SRC_CONFIG_FILE})
SOURCE_GROUP(Impore/Asset/Base FILES ${IMPORT_SRC_ASSET_BASE_FILE})

if (MSVC)
    add_compile_options("/utf-8")
endif()


project(ResMgr)

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
	${IMPORT_SRC_FILE}
	${IMPORT_SRC_CONFIG_FILE}
	${IMPORT_SRC_ASSET_FILE}
	${IMPORT_SRC_ASSET_BASE_FILE}
	${INCLUDE_HFILES}
	${IMGUI_SRC}
	tiny-json.h
	tiny-json.c
	tinyxml2.h
	tinyxml2.cpp
	
	${IMPORT_DIR}/tiny_obj_loader.cc
	
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
  ${IMPORT_DIR}
)

add_executable(ResMgr ${SRC_FILES})

if(WIN32)
	target_link_libraries(ResMgr PRIVATE ${DEPENDENCY_FILES})	
endif(WIN32)
		
if(APPLE)
	target_link_libraries(ResMgr PRIVATE  ${DEPENDENCY_FILES})
endif(APPLE)





