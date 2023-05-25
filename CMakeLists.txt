# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (threadpool)

# 查找目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(../src DIR_SRCS)
set(CMAKE_CXX_FLAGS "-g -pthread -std=c++17")
# 指定生成目标
add_executable(./bin/threadpool ${DIR_SRCS})
