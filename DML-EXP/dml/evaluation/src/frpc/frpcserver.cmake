


include_directories(${frpcsrcdir}/../)

find_library(LIBUV libuv.a ${frpcsrcdir}/../)

add_library(libfrpcserver

    STATIC

    ${frpcsrcdir}/../encyptwrapper/crc32.cpp
    ${frpcsrcdir}/../publicfun/quicklz/quicklz.c
    ${frpcsrcdir}/../publicfun/debug.c
    ${frpcsrcdir}/../publicfun/thrqueue.c
    ${frpcsrcdir}/rpchead.cpp
    ${frpcsrcdir}/tcpserver.cpp
    ${frpcsrcdir}/rpctcpserver.cpp

)

target_link_libraries(libfrpcserver ${LIBUV} pthread)

