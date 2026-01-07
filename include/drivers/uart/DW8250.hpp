#pragma once

template <unsigned long A, unsigned long C, unsigned long B> struct DW8250 {

    static volatile unsigned char &LSR() { return *reinterpret_cast<unsigned char *>(A + 0x14); }
    static volatile unsigned char &TXD() { return *reinterpret_cast<unsigned char *>(A); }

    enum { TX_EMPTY = 1ULL << 5 };

    static void init() {}

    static void put(char c) {
        while (!(LSR() & TX_EMPTY))
            ;
        TXD() = c;
    }
};
