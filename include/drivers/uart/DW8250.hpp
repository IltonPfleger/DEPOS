#pragma once

#include <drivers/Driver.hpp>
#include <utils/Observer.hpp>

template <unsigned long Address> struct DW8250 : Driver, public Observed<unsigned char *, size_t> {

    enum Registers {
        THR = 0,
        LSR = 20,
    };

    enum Bits {
        LSR_TX_EMPTY = 1ULL << 5,
    };

    static DW8250 *instance() {
        static DW8250 _;
        return &_;
    };

    static void init() {}

    void putc(char c) {
        while ((Reg8(Address, LSR) & LSR_TX_EMPTY) == 0)
            ;
        Reg8(Address, THR) = c;
    }
};
