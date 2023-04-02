CMAKE_MINIMUM_REQUIRED(VERSION 3.26.1)
PROJECT(sl_fmt)

SET(SRC src/sl_fmt.cpp)
SET(HDR include/sl_fmt.hpp)

ADD_LIBRARY(sl_fmt ${SRC} ${HDR})
TARGET_INCLUDE_DIRECTORIES(sl_fmt PRIVATE "include")
SET_PROPERTY(TARGET sl_fmt PROPERTY CXX_STANDARD 14)

IF(BUILD_TESTS)
    ENABLE_TESTING()

    SET(TEST_SRC tests/LiteralTest.cpp)
    SET(TEST_HDR tests/LiteralTest.hpp)

    ADD_EXECUTABLE(LiteralTest ${TEST_SRC} ${TEST_HDR})
    TARGET_INCLUDE_DIRECTORIES(LiteralTest PRIVATE "include" PUBLIC "/home/philipp/Libraries/doctest/doctest/")
    TARGET_LINK_LIBRARIES(LiteralTest sl_fmt)
    ADD_TEST(NAME LiteralTest COMMAND LiteralTest)
ENDIF()
