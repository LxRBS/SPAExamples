# 这一行是说把这些文件构建成一个可执行文件main
add_executable( main main.cpp )

# 这一行是说构建可执行文件要用到的库
target_link_libraries( main ${llvm_libs} ${svf_lib})
