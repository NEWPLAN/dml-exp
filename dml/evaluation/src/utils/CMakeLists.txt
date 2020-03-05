aux_source_directory(. DIR_LIB_SRCS)
MESSAGE("building myutils with ${DIR_LIB_SRCS}")
add_library(myutils SHARED ${DIR_LIB_SRCS})