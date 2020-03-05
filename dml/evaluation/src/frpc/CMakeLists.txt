cmake_minimum_required(VERSION 3.0.0)
project(testrpc VERSION 0.1.0)

include_directories(..)



add_library(libfrpcclient
    ../publicfun/debug.c
    ../publicfun/thrqueue.c
    ../publicfun/condmutex.cpp

    ../threadwrapper/threadwrapper.cpp
    ../encyptwrapper/crc32.cpp
    ../socketwrapper/socketWrapper.c
    ../publicfun/quicklz/quicklz.c
    ../publicfun/filesystemwrapper.cpp

    rpchead.cpp
    tcpclient.cpp
    msgstub.cpp
    )

add_executable(tsttcp
    ../publicfun/public.cpp
    tsttcp.cpp
)

target_link_libraries(tsttcp libfrpcclient pthread)


set(frpcsrcdir ${PROJECT_SOURCE_DIR})

include(frpcserver.cmake)

add_executable(tcpserver
    ../publicfun/public.cpp
    tsttcpserver.cpp
)


target_link_libraries(tcpserver   libfrpcserver  pthread)


