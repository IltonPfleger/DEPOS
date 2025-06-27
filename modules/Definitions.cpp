export module Definitions;

template <bool B, typename True, typename False>
struct TypeSelector {
    using Type = True;
};

template <typename True, typename False>
struct TypeSelector<false, True, False> {
    using Type = False;
};

export typedef TypeSelector<sizeof(void*) == 4, long long, int>::Type intptr_t;
export typedef TypeSelector<sizeof(void*) == 8, unsigned long long, unsigned>::Type uintptr_t;

export struct Machine {
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
