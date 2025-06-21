#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

struct Machine {
    static constexpr const unsigned int XLEN = 64;
    static constexpr const unsigned int CPUS = 1;
    struct Memory {
        static constexpr const unsigned int ORDER = 30;
        static constexpr const unsigned int SIZE  = (1 << ORDER);
        struct Page {
            static constexpr const unsigned int ORDER = 12;
            static constexpr const unsigned int SIZE  = (1 << ORDER);
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

using intptr_t  = TypeSelector<Machine::XLEN == 64, long long, int>::Type;
using uintptr_t = TypeSelector<Machine::XLEN == 64, unsigned long long, unsigned>::Type;

#endif
