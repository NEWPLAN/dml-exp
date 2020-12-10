# 按惯例，cmake的版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
# 项目名称
PROJECT(datasubsystem)

find_package(Threads)

# C++和CUDA的编译参数，可选。
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Os -Wall")
# 头文件路径，按需
INCLUDE_DIRECTORIES(
    ./)


# 库文件路径，按需
#LINK_DIRECTORIES(/usr/lib
#    /usr/local/lib)



ADD_EXECUTABLE(data_benckmark
    ./example/test_datasystem.cpp
)


ADD_LIBRARY(datasys SHARED 
                data_generator.cpp
            )
TARGET_LINK_LIBRARIES(datasys ${CMAKE_THREAD_LIBS_INIT})
# 链接外部库，按需
TARGET_LINK_LIBRARIES(data_benckmark 
                        ${CMAKE_THREAD_LIBS_INIT}
                        datasys)
