cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_SCAN_FOR_MODULES ON)

set(NAME draw_compute)

set(SRC_DIR ../${NAME})
set(BASE_DIR ../base)
set(DEP_DIR "../../../deps")
set(MAIN_DIR "../../../")
set(STB_DIR "../../../deps/stb")
set(GLM_DIR "../../../deps/glm")
set(IMGUI_DIR "../../../deps/imgui")
set(GLSLANG_DIR "../../../deps/glslang")
set(TINYGLTF_DIR "../../../deps/tinygltf")
set(TASKFLOW_DIR "../../../deps/taskflow")
set(SRC_DIR ${MAIN_DIR}/Unit_Test/draw_compute)
set(RENDER_DIR ${MAIN_DIR}/Render)

file(GLOB EXAMPLE_SRC ${SRC_DIR}/*.cpp ${SRC_DIR}/include/*.h ${SRC_DIR}/src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DVK_USE_PLATFORM_ANDROID_KHR -DVK_NO_PROTOTYPES")

add_library(native-lib SHARED ${EXAMPLE_SRC})

add_library(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

add_subdirectory(../base ${CMAKE_SOURCE_DIR}/../base)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

include_directories(${BASE_DIR})
include_directories(${MAIN_DIR})
include_directories(${DEP_DIR})
include_directories(${DEP_DIR}/glm)
include_directories(${SRC_DIR}/include)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(
    native-lib
    native-app-glue
    RenderLib
    android
    log
    z
)
