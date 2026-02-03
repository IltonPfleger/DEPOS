#pragma once

#include <drivers/Driver.hpp>

template <unsigned long Base> class SynopsysDW8250 : Driver {

    enum Registers {
        TXD = 0x00,
        LSR = 0x05,
    };

    enum Bits { TEMT = 1ULL << 5 };

  public:
    static void init() {}
    static void put(char c) {
        while (!(Reg8(Base, LSR) & TEMT))
            ;
        Reg8(Base, TXD) = c;
    }
};
