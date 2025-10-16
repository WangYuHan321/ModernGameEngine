
export curPath=${PWD}
export androidPath=${curPath}/android

cd ${androidPath}
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE="${ANDROID_HOME}/ndk/27.0.12077973/build/cmake/android.toolchain.cmake" -DANDROID_ABI=arm64-v8a ${androidPath}/../
