#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

struct Machine {
    static constexpr const char* ARCH    = "RISCV";
    static constexpr const uint32_t XLEN = 64;
    static constexpr const uint32_t CPUS = 4;
    struct Memory {
        static constexpr const uint32_t ORDER = 30;
        static constexpr const uint32_t SIZE  = (1 << ORDER);
        struct Page {
            static constexpr const uint32_t ORDER = 12;
            static constexpr const uint32_t SIZE  = (1 << ORDER);
        };
    };
};

template <bool B, typename True, typename False>
struct TypeSelector {
    using Type = True;
};

template <typename True, typename False>
struct TypeSelector<false, True, False> {
    using Type = False;
};

using intptr_t  = TypeSelector<Machine::XLEN == 64, int64_t, int32_t>::Type;
using uintptr_t = TypeSelector<Machine::XLEN == 64, uint64_t, uint32_t>::Type;

#endif
