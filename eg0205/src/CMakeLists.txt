# 这一行是说把这些文件构建成一个可执行文件main
add_executable( main main.cpp )

# 这一行是说构建可执行文件要用到的库
# 先写SVF库，再写llvm库，因为可能顺序是反着的（并不确定）
target_link_libraries( main ${svf_lib} ${llvm_libs})