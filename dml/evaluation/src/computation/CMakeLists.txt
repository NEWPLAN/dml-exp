# 按惯例，cmake的版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
# 项目名称
PROJECT(AD-Census)
# cmake寻找cuda，这个要现在系统里面装好cuda，设置好cuda的环境参数啥的
FIND_PACKAGE(CUDA REQUIRED)
# C++和CUDA的编译参数，可选。
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
SET(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_50,code=sm_50; -gencode arch=compute_52,code=sm_52; -gencode arch=compute_60,code=sm_60 ;-gencode arch=compute_61,code=sm_61;-G;-g)
# 头文件路径，按需
INCLUDE_DIRECTORIES(
    ./)
# 库文件路径，按需
LINK_DIRECTORIES(/usr/lib
    /usr/local/lib)
# 主要就是这个，教cmake去找nvcc来编译这些东西
CUDA_ADD_EXECUTABLE(ad-census
    #main.cu
    tensor_merge.cu
    ./example/example.cc
)

CUDA_ADD_LIBRARY(cumerge SHARED 
                tensor_merge.cu 
                tensor_merge.cpp
                testcase.cpp)

# 链接外部库，按需
TARGET_LINK_LIBRARIES(ad-census)