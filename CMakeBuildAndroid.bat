SET build_android="%~dp0\test"

echo 123456
echo %ANDROID_HOME%

cd %build_android%

cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=%ANDROID_HOME%/ndk/27.0.12077973/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a ../
