cmake_minimum_required(VERSION 3.13.4)
project("z3instance")

# 设置z3
set(Z3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/z3_dist)
set(Z3_INCLUDES ${Z3_DIR}/include)
set(Z3_LIB ${Z3_DIR}/lib)

# 指定项目的inlucde与link目录
include_directories(${Z3_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/include)
LINK_DIRECTORIES(${Z3_LIB})

# 添加编译项目
add_subdirectory(src)
