cmake_minimum_required(VERSION 3.10)  # 说明本项目所需的cmake版本

# 本项目的名称，与最后生成的可执行程序的名称可以没有任何关系
project("FirstCMake")  

# 说明本项目中有一个名为main的可执行文件，是依赖于main.c和f.c生成的
add_executable(main main.c f.c)
