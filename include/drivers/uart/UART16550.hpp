#pragma once

template <unsigned long Addr> class UART16550 {
    static volatile char &THR() { return *reinterpret_cast<volatile char *>(Addr + 0); };
    static volatile char &IER() { return *reinterpret_cast<volatile char *>(Addr + 1); };
    static volatile char &FCR() { return *reinterpret_cast<volatile char *>(Addr + 2); };
    static volatile char &LCR() { return *reinterpret_cast<volatile char *>(Addr + 3); };
    static volatile char &MCR() { return *reinterpret_cast<volatile char *>(Addr + 4); };
    static volatile char &LSR() { return *reinterpret_cast<volatile char *>(Addr + 5); };
    static volatile char &LSB() { return *reinterpret_cast<volatile char *>(Addr + 0); }; // Divisor Latch LSB
    static volatile char &MSB() { return *reinterpret_cast<volatile char *>(Addr + 1); }; // Divisor Latch MSB
    static constexpr char LSR_TX_EMPTY = (1 << 5);

  public:
    static void put(char c) {
        while ((LSR() & LSR_TX_EMPTY) == 0)
            ;
        THR() = c;
    }
};
