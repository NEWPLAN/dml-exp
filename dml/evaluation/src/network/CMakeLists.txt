# 按惯例，cmake的版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
# 项目名称
PROJECT(networksubsystem)

find_package(Threads)

# C++和CUDA的编译参数，可选。
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Os")
# 头文件路径，按需
INCLUDE_DIRECTORIES(
    ./tcp/
    ./rdma/
    ./
    )

# aux_source_directory(
#     ./tcp/
#     ./rdma/
#     ./
# )

# 库文件路径，按需
#LINK_DIRECTORIES(/usr/lib
#    /usr/local/lib)

# 主要就是这个，教cmake去找nvcc来编译这些东西
ADD_EXECUTABLE(server
    ./example/example_server.cpp
)
ADD_EXECUTABLE(client
    ./example/example_client.cpp
)

#ADD_LIBRARY(cumerge SHARED 
#                tensor_merge.cu 
#                testcase.cpp)

ADD_LIBRARY(netsys SHARED 
                ./tcp/client.cpp
                ./tcp/server.cpp
                tower.cpp
                )

TARGET_LINK_LIBRARIES(netsys ${CMAKE_THREAD_LIBS_INIT})

# 链接外部库，按需
TARGET_LINK_LIBRARIES(server 
                        ${CMAKE_THREAD_LIBS_INIT}
                        netsys)
TARGET_LINK_LIBRARIES(client 
                        ${CMAKE_THREAD_LIBS_INIT}
                        netsys)
