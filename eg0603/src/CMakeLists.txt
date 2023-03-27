# 生成可执行文件的源文件
add_executable(main main.cpp)
# z3是已经弄好的外部库，直接链接到可执行文件上即可
target_link_libraries(main z3)