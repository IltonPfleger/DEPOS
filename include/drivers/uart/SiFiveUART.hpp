#include <Types.hpp>

struct SiFiveUART {
    static constexpr uintptr_t Addr = 0x10010000;
    static constexpr int Clock      = 31250000;
    static constexpr int Baudrate   = 115200;
    static constexpr int Divisor    = Clock / Baudrate;

    static volatile unsigned int *base() { return reinterpret_cast<volatile unsigned int *>(Addr); }

    static volatile unsigned int &TXDATA() { return *(base() + 0); }
    static volatile unsigned int &RXDATA() { return *(base() + 1); }
    static volatile unsigned int &TXCTRL() { return *(base() + 2); }
    static volatile unsigned int &RXCTRL() { return *(base() + 3); }
    static volatile unsigned int &IE() { return *(base() + 4); }
    static volatile unsigned int &IP() { return *(base() + 5); }
    static volatile unsigned int &DIV() { return *(base() + 6); }

    static constexpr unsigned int TX_EMPTY_MASK = (1 << 31);
    static constexpr unsigned int RX_EMPTY_MASK = 0x80000000;

    static void init() {
        DIV()    = Divisor;
        TXCTRL() = 1;
        RXCTRL() = 1;
        IE()     = 0;
    }

    static void put(char c) {
        while (TXDATA() & TX_EMPTY_MASK);
        TXDATA() = c;
        while (TXDATA() & TX_EMPTY_MASK);
    }
};
