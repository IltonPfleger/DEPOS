#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <utils/meta.hpp>

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

struct Machine {
    static constexpr const char* ARCH = "RISCV";
    static constexpr uint32_t XLEN    = 64;
    static constexpr uint32_t CPUS    = 4;
    struct Memory {
        static constexpr const uint32_t order = 30;
        static constexpr const uint32_t size  = (1 << order);
    };
};

using intptr_t  = Meta::TypeSelector<Machine::XLEN == 64, int64_t, int32_t>::Type;
using uintptr_t = Meta::TypeSelector<Machine::XLEN == 64, uint64_t, uint32_t>::Type;

#endif
