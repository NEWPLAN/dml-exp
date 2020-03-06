CMAKE_MINIMUM_REQUIRED(VERSION 3.0) #cmake版本要求
project(Hello)

message("Building make for Hello") #打印消息
message("add -DDYNAMIC_E=1 when build dynamic class E, the default is static")
set(CMAKE_BUILD_TYPE "Debug")

set(SUB_MODULE_PATH "./utils")
set(CMAKE_CXX_STANDARD 11) # 设置std=c++11

# 设置debug或者release的编译参数
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Os -Wall -fPIC")

# collect all message to "SRC_LIST" <-- ./*.cpp
aux_source_directory(./main SRC_LIST)

#分别指定编译include和链接link的搜索目录
include_directories(${SUB_MODULE_PATH} ./computation)
link_directories(${SUB_MODULE_PATH}  ./computation)

#添加一个子模块，编译到lib目录下去
add_subdirectory(${SUB_MODULE_PATH}) # build uitls subsystem
add_subdirectory(./computation) # build computation subsystem
add_subdirectory(./network) # build network subsystem
add_subdirectory(./datasystem) # build data subsystem

#设置编译目标
add_executable(Hello ${SRC_LIST})
#添加编译目标依赖
target_link_libraries(Hello 
                    myutils 
                    cumerge 
                    netsys
                    datasys
                    pthread )