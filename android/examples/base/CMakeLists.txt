file(GLOB BASE_SRC "./*.cpp" "./*.h" "./*.hpp" "../../../../dps/imgui/*.cpp")

add_library(libbase SHARED ${BASE_SRC})

set (LIB_INC ${BASE_DIR}
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