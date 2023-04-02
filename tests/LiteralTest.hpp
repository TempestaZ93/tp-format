#include <cstdio>
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "sl_fmt.hpp"

struct TestStruct {
    int val1;
    float val2;
};
std::ostream &operator<<(std::ostream &os, TestStruct testStruct) {
    return os << "[val1: " << testStruct.val1 << ", val2: " << testStruct.val2 << "]";
}

TEST_CASE("Simple Literal") {
    format("Test and stuff {} {} but also {0x} and {0X} and {0b} and {0o} or {.3}", "and this", 252,
           252, 252, 252, 252, 3.1234123);

    int *val = new int[1];
    format("Pointer address = {}", val);

    format("This {} is a object.", TestStruct{1, 2.0});
}
