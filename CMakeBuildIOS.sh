#mkdir ios

export curPath=${PWD}

export srcIconPath=$curPath/Asset/res/icon
export dstIconPath=$curPath/ios/ios/Resource

export srcPath=$curPath/Asset
export dstPath=$curPath/ios

cp ${srcIconPath}/*.* ${dstIconPath}
cp -r ${srcPath} ${dstPath}

#copy some project
export unitProject=$curPath/Unit_Test

cp -r ${unitProject} ${dstPath}


# export VULKAN_SDK=/Users/admin/VulkanSDK/1.4.321.0/macOS
# export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib
# export VK_ADD_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
# export VK_ICD_FILENAMES=$VULKAN_SDK/share/vulkan/icd.d/MoltenVK_icd.json
# export VK_DRIVER_FILES=$VK_ICD_FILENAMES
# export PKG_CONFIG_PATH=$VULKAN_SDK/lib/pkgconfig
# export Vulkan_INCLUDE_DIR=$VULKAN_SDK/include
# export Vulkan_LIBRARY=$VULKAN_SDK/lib


# cmake -GXcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=x86_64 \
#    -DVULKAN_SDK:PATH=$VULKAN_SDK -DDYLD_LIBRARY_PATH:PATH=$DYLD_LIBRARY_PATH \
#    -DVK_ADD_LAYER_PATH:PATH=$VK_ADD_LAYER_PATH -DVK_ICD_FILENAMES:PATH=$VK_ICD_FILENAMES \
#    -DVK_DRIVER_FILES:PATH=$VK_DRIVER_FILES -DPKG_CONFIG_PATH:PATH=$PKG_CONFIG_PATH \
#    -DVulkan_INCLUDE_DIR:PATH=$Vulkan_INCLUDE_DIR -DVulkan_LIBRARY:PATH=$Vulkan_LIBRARY -B build -S ../
