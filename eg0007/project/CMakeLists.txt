cmake_minimum_required(VERSION 3.4.3)  # 指定cmake的最小版本
project("NotImportant")                # 项目名称，与后面可执行文件的名称没有必然联系

# 注释其中一行，可以在Debug版本与Release版本之间切换
# 还可以设置不同的后续参数
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

# 这一行是说要去src目录中进行编译，编译的结果放在bin目录里
add_subdirectory(src bin)

# 输出一些信息
message(STATUS "The root of project is " ${PROJECT_SOURCE_DIR})
