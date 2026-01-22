#pragma once

#include <drivers/Driver.hpp>

template <unsigned long Addr> struct SiFiveUART : Driver {
    // static volatile unsigned int &TXDATA() { return *(Addr + 0); }
    // static volatile unsigned int &RXDATA() { return *(Addr + 1); }
    // static volatile unsigned int &TXCTRL() { return *(Addr + 2); }
    // static volatile unsigned int &RXCTRL() { return *(Addr + 3); }
    // static volatile unsigned int &IE() { return *(Addr + 4); }
    // static volatile unsigned int &IP() { return *(Addr + 5); }
    // static volatile unsigned int &DIV() { return *(Addr + 6); }
    // static constexpr unsigned int RX_EMPTY_MASK = 0x80000000;
    static constexpr unsigned int TX_EMPTY_MASK = (1 << 31);

    enum Registers { TXDATA = 0 };

    static void put(char c) {
        while (Reg32(Addr, TXDATA) & TX_EMPTY_MASK)
            ;
        Reg32(Addr, TXDATA) = c;
    }
};
