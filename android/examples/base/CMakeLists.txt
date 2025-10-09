file(GLOB BASE_SRC "./*.cpp" "./*.h" "./*.hpp" "../../../../dps/imgui/*.cpp")

add_library(libbase SHARED ${BASE_SRC})

include_directories(${BASE_DIR})
include_directories(../../../../deps")
include_directories(../../../../deps/glm")
include_directories(../../../../deps/stb")
include_directories(../../../../deps/glfw")
include_directories(../../../../deps/imgui")
include_directories(../../../../deps/glslang")
include_directories(../../../../deps/tinygltf")
include_directories(../../../../deps/taskflow")
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(
	libbase
	android
	log
	z
)