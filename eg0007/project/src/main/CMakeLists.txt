# 用main.c生成名为main的可执行文件
add_executable(main main.c)

# 生成可执行文件main需要用到的funlibs库
target_link_libraries(main funlibs)
