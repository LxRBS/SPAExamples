# 用fun.c生成一个名为funlibs的动态库
add_library(funlibs STATIC fun.c)

# 生成funlibs需要include这些路径，这里采用了绝对路径和相对路径相结合的方式，一般应该采用环境变量
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/../../eg0005extern/includes")

# 生成funlibs要用到这些路径的库，其中m库是C语言的数学库，gcc并没有默认包含这个库，因此需要手工指定
target_link_libraries(funlibs PUBLIC "${PROJECT_SOURCE_DIR}/../../eg0005extern/libs/liblxmath.so" m)
