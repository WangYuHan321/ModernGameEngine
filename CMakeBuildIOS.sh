#mkdir ios

export curPath=${PWD}

export srcIconPath=$curPath/Asset/res/icon
export dstIconPath=$curPath/ios/ios/Resource

export srcPath=$curPath/Asset
export dstPath=$curPath/ios

cp ${srcIconPath}/*.* ${dstIconPath}
cp -r ${srcPath} ${dstPath}


cd ios
cmake -GXcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DWEBP_BUILD_DWEBP=0 -DWEBP_BUILD_CWEBP=0 \
  -DWEBP_BUILD_IMG2WEBP=0 -DWEBP_BUILD_WEBPINFO=0 \
  -DWEBP_BUILD_WEBPMUX=0 -DWEBP_BUILD_EXTRAS=0 \
  -DENABLE_OPT=0
