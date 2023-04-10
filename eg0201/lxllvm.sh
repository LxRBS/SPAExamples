#! /bin/bash

# 本脚本用于给定.c文件，同时生成ll文件、bc文件和opt文件

# 判断参数数量，必须是1个参数
if [ $# -eq 0 ];then
    echo "Execution Failure!"
    echo "There must be ONE argument, but NO argument found."
    exit 1
elif [ $# -gt 1 ];then
    echo "Execution Failure!"
    echo "There must be ONE argument, but $# arguments found."
    exit 1
fi

# 获取文件名以及后缀名
full_filename=$1
filename="${full_filename%.*}"
extension="${full_filename##*.}"

# 必须是c文件
if [ "$extension" != "c" ]; then
    echo "Execution Failure!"
    echo "Only c file is available, but $full_filename is not a c file."
    exit 1
fi

full_version=$(llvm-as --version | grep "LLVM version")
if [ -z "${full_version}" ]; then
    echo "LLVM version does NOT exist, ERROR!"
    exit 1
fi

real_version="${full_version##* }"
if [ -z "${real_version}" ]; then
    echo "LLVM version number cant be get from $full_version, ERROR!"
    exit 1
fi

real_version_number="${real_version%%.*}"  # 从左往右截取第一个'.'，右边的内容舍去，左边的内容保留
if [ -z "${real_version_number}" ]; then
    echo "LLVM major version number cant be get from $real_version, ERROR!"
    exit 1
fi

echo Compile and link "$full_filename" with LLVM "$real_version_number"

opaq_parm=""
if [ $real_version_number -ge 15 ]; then
    opaq_parm="-Xclang -no-opaque-pointers"
fi

# 生成同名的ll文件
# -disable-O0-optnone 
clang -S -c ${opaq_parm} -fno-discard-value-names -g -emit-llvm "$full_filename" -o "$filename"".ll" 
# opt -S -mem2reg "$filename"".ll" -o "$filename"".ll"

# 生成同名的bc文件
llvm-as "$filename"".ll" -o "$filename"".bc"

# 生成同名的opt文件
opt -mem2reg "$filename"".bc" -o "$filename"".opt"

# to create the directory with the same name and move files
mkdir -p "./""$filename"
mv -f "$filename"".bc" "$filename"".opt" "$filename"".ll" "./""$filename"
# src file needs copy
cp -f "$filename"".c" "./""$filename"
