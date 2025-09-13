#!/bin/sh

export curPath=${PWD}/../deps
export curDepPath=$curPath/deps

export curGlslangPath=$curDepPath/glslang


# 检查目录是否存在
if [ -d "$curGlslangPath" ]; then
    echo "目录存在: $curGlslangPath"
    # 如果目录存在，可以执行一些操作，例如列出目录内容
    ls "$directory_name"
else
    echo "目录不存在: $curGlslangPath"
    # 如果目录不存在，可以创建目录或执行其他操作
    mkdir -p "$curGlslangPath" && echo "已创建目录: $curGlslangPath"
fi



cd $curGlslangPath

cmake ..
make

