# CMake control-file for run-time timer classes
# RW Penney, December 2017

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(rtimers)
ENABLE_TESTING()

FIND_PACKAGE(Threads)

IF(CMAKE_COMPILER_IS_GNUCC)
    ADD_DEFINITIONS(-ansi -std=c++11 -pedantic -Wall)
    SET(CMAKE_CXX_FLAGS_DEBUG:STRING "-ggdb")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

FIND_PACKAGE(Boost 1.60 COMPONENTS system thread unit_test_framework)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    ADD_DEFINITIONS(-DRTIMERS_HAVE_BOOST=1)
ENDIF(Boost_FOUND)


SET(lib_hdrs
    rtimers/boost.hpp
    rtimers/core.hpp
    rtimers/cxx11.hpp
    rtimers/posix.hpp
)

SET(test_srcs
    testboost.cpp
    testcxx11.cpp
    testmain.cpp
    testposix.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})


INSTALL(FILES ${lib_hdrs} DESTINATION include/rtimers)

ADD_EXECUTABLE(demo ${lib_hdrs} demo.cpp)
TARGET_LINK_LIBRARIES(demo ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
SET_TARGET_PROPERTIES(demo
    PROPERTIES ADDITIONAL_CLEAN_FILES "rtimers-demo.log")


IF(Boost_FOUND)
    ADD_EXECUTABLE(timer_tests ${lib_hdrs} testdefns.hpp ${test_srcs})
    SET_TARGET_PROPERTIES(timer_tests
        PROPERTIES
            COMPILE_FLAGS "-DUNIT_TESTING -DBOOST_TEST_DYN_LINK")
    TARGET_LINK_LIBRARIES(timer_tests ${Boost_LIBRARIES}
                          ${CMAKE_THREAD_LIBS_INIT})
    ADD_TEST(TT timer_tests)
ENDIF(Boost_FOUND)
