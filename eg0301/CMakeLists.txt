cmake_minimum_required(VERSION 3.4.3)  # 指定cmake的最小版本
project("NotImportant")                   # 项目名称，与后面可执行文件的名称没有必然联系

# 以下为设置gcc编译指令的选项
if(DEFINED ENV{LLVM_DIR})
    set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_FLAGS "-fPIC -std=gnu++14 -O3 -fno-rtti -Wno-deprecated -g")
set(CMAKE_C_FLAGS "-fPIC")

# 这两行是关键，一行指定外部库，一行指定include文件
find_package(LLVM REQUIRED CONFIG)        
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

# 这一行是把所有的LLVM库集中用一个名字来代替
llvm_map_components_to_libnames(llvm_libs BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support TransformUtils)

# 这两行用SVF
# 这一行指定SVF的库
set(svf_lib "$ENV{SVF_DIR}/Debug-build/lib/libSvf.a")
include_directories(SYSTEM "$ENV{SVF_DIR}/include")

# 这一行指定要去构建src
add_subdirectory(src bin)

