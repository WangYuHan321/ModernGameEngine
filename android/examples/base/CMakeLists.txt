cmake_minimum_required(VERSION 3.13)

set(DEP_DIR "../../../deps")
set(MAIN_DIR "../../../")
set(STB_DIR "../../../deps/stb")
set(GLM_DIR "../../../deps/glm")
set(IMGUI_DIR "../../../deps/imgui")
set(GLSLANG_DIR "../../../deps/glslang")
set(TINYGLTF_DIR "../../../deps/tinygltf")
set(TASKFLOW_DIR "../../../deps/taskflow")

if(EXISTS ${IMGUI_DIR}/imgui.cpp)
	# 文件存在时执行的操作
	message("文件存在！")

endif()

if(NOT EXISTS ${IMGUI_DIR}/imgui.cpp)
	# 文件存在时执行的操作
	message("文件不存在！")
	message(${IMGUI_DIR})

endif()

set (IMGUI_SRC ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
file(GLOB RENDER_SRC "../../../Render/*.h" "../../../Render/*.cpp" "../../../Render/*.hpp")
file(GLOB VULKAN_SRC "../../../Render/Vulkan/*.h" "../../../Render/Vulkan/*.cpp" "../../../Render/Vulkan/*.hpp")

SOURCE_GROUP(Render FILES ${RENDER_SRC})
SOURCE_GROUP(Render/Vulkan FILES ${VULKAN_SRC})

add_library(RenderLib SHARED ${RENDER_SRC} ${VULKAN_SRC} ${IMGUI_SRC})

set (LIB_INC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}
		${DEP_DIR}
		${GLM_DIR}
		${STB_DIR}
		${IMGUI_DIR}
		${GLSLANG_DIR}
		${TINYGLTF_DIR}
		${TASKFLOW_DIR}
		${ANDROID_NDK}/sources/android/native_app_glue
)

include_directories(${LIB_INC})

target_link_libraries(
	RenderLib
	android
	log
	z
)