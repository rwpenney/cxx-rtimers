# CMake control-file for run-time timer classes
# RW Penney, December 2017

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
PROJECT(rtimers)
ENABLE_TESTING()


IF(CMAKE_COMPILER_IS_GNUCC)
    ADD_DEFINITIONS(-ansi -pedantic -Wall)
    SET(CMAKE_CXX_FLAGS_DEBUG:STRING "-ggdb")
ENDIF(CMAKE_COMPILER_IS_GNUCC)


FIND_PACKAGE(Boost 1.48 COMPONENTS system unit_test_framework)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})


SET(lib_hdrs
    core.hpp
)


INSTALL(FILES ${lib_hdrs} DESTINATION include/rtimers)

ADD_EXECUTABLE(demo ${lib_hdrs} demo.cpp)

IF(Boost_FOUND)
    ADD_EXECUTABLE(timer_tests ${lib_hdrs} testmain.cpp)
    SET_TARGET_PROPERTIES(timer_tests
        PROPERTIES
            COMPILE_FLAGS "-DUNIT_TESTING -DBOOST_TEST_DYN_LINK")
    TARGET_LINK_LIBRARIES(timer_tests ${Boost_LIBRARIES})
    ADD_TEST(TT timer_tests)
ENDIF(Boost_FOUND)
