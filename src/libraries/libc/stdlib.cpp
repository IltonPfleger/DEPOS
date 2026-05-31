#include <libraries/libc/stdlib.h>
#include <memory/Heap.hpp>

using namespace DEPOS;

extern "C" void *malloc(size_t size) { return new uint8_t[size]; }

inline long atol(const char *str) {
    long result   = 0;
    bool negative = false;

    if (*str == '-') {
        negative = true;
        ++str;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        ++str;
    }

    return negative ? -result : result;
}
