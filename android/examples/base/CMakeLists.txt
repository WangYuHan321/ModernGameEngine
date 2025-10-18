file(GLOB BASE_SRC "../../../dps/imgui/*.cpp" "./*.h" "./*.cpp")

file(GLOB RENDER_SRC "../../../Render/*.h" "../../../Render/*.cpp" "../../../Render/*.hpp")
file(GLOB VULKAN_SRC "../../../Render/Vulkan/*.h" "../../../Render/Vulkan/*.cpp" "../../../Render/Vulkan/*.hpp")

add_library(libbase SHARED ${BASE_SRC} ${RENDER_SRC} ${VULKAN_SRC})

SOURCE_GROUP(Render FILES  ${RENDER_SRC})
SOURCE_GROUP(Render/Vulkan FILES  ${VULKAN_SRC})

set (LIB_INC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}
		../../../deps
		../../../deps/glm
		../../../deps/stb
		../../../deps/glfw
		../../../deps/imgui
		../../../deps/glslang
		../../../deps/tinygltf
		../../../deps/taskflow
		${ANDROID_NDK}/sources/android/native_app_glue
)

include_directories(${LIB_INC})

target_link_libraries(
	libbase
	android
	log
	z
)