#pragma once

#include <drivers/Driver.hpp>

template <unsigned long Base> class SynopsysDW8250 : Driver {

    enum Registers {
        TXD = 0,
        LSR = 0x14,
    };

    enum Bits { TX_EMPTY = 1ULL << 5 };

  public:
    static void init() {}
    static void put(char c) {
        while (!(Reg8(Base, LSR) & TX_EMPTY))
            ;
        Reg8(Base, TXD) = c;
    }
};
