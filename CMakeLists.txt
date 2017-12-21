# CMake control-file for run-time timer classes
# RW Penney, December 2017

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
PROJECT(rtimers)

IF(CMAKE_COMPILER_IS_GNUCC)
    ADD_DEFINITIONS(-ansi -pedantic -Wall)
    SET(CMAKE_CXX_FLAGS_DEBUG:STRING "-ggdb")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

SET(lib_hdrs
    core.hpp
)


INSTALL(FILES ${lib_hdrs} DESTINATION include/rtimers)

ADD_EXECUTABLE(demo ${lib_hdrs} demo.cpp)
