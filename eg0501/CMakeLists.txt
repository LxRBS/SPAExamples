cmake_minimum_required(VERSION 3.4.3)  # 指定cmake的最小版本
project("NotImportant")                   # 项目名称，与后面可执行文件的名称没有必然联系

# 以下为设置gcc编译指令的选项
if(DEFINED ENV{LLVM_DIR})
    set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_FLAGS "-fPIC -std=gnu++14 -O3 -fno-rtti -Wno-deprecated")
set(CMAKE_C_FLAGS "-fPIC")

set(CMAKE_BUILD_TYPE "Debug")

# 首先判断一下是否有LLVM库
if(DEFINED ENV{LLVM_DIR})
    message(STATUS "ENV:LLVM_DIR = $ENV{LLVM_DIR}")
else()
    message(FATAL_ERROR "ENV LLVM_DIR not found!")
endif()

# 这两行是关键，一行指定外部库，一行指定include文件
find_package(LLVM REQUIRED CONFIG)        
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

# 这一行是把所有的LLVM库集中用一个名字来代替
llvm_map_components_to_libnames(llvm_libs BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support TransformUtils)

# 首先判断一下SVF是否存在
if(DEFINED ENV{SVF_DIR})
    message(STATUS "ENV:SVF_DIR = $ENV{SVF_DIR}")
else()
    message(FATAL_ERROR "ENV SVF_DIR not found!")
endif()

# 直接用环境变量指定
set(svf_lib "$ENV{SVF_DIR}/Debug-build/lib/libSvf.a")
include_directories(SYSTEM "$ENV{SVF_DIR}/include")

# 这一行指定要去构建src
add_subdirectory(src bin)

