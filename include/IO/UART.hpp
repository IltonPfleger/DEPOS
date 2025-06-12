#ifndef UART_HPP
#define UART_HPP

template <uintptr_t BaseAddr, uint32_t Clock, uint32_t Baudrate>
struct UART {
    static volatile uint8_t& THR() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 0); };
    static volatile uint8_t& IER() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 1); };
    static volatile uint8_t& FCR() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 2); };
    static volatile uint8_t& LCR() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 3); };
    static volatile uint8_t& MCR() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 4); };
    static volatile uint8_t& LSR() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 5); };
    static volatile uint8_t& LSB() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 0); };  // Divisor Latch LSB
    static volatile uint8_t& MSB() { return *reinterpret_cast<volatile uint8_t*>(BaseAddr + 1); };  // Divisor Latch MSB
    static constexpr uint8_t LSR_TX_EMPTY = (1 << 5);
    static constexpr uint64_t DIVISOR     = (Clock + Baudrate * 8) / (Baudrate * 16);

    static void init() {
        IER() = 0x00;
        LCR() = (1 << 7);
        LSB() = DIVISOR & 0xFF;
        MSB() = (DIVISOR >> 8) & 0xFF;
        LCR() = 0x03;
        FCR() = 0x07;
        IER() = 0x01;
        while ((LSR() & LSR_TX_EMPTY) == 0);
    }

    static void put(char c) {
        while ((LSR() & LSR_TX_EMPTY) == 0);
        THR() = c;
    }
};

#endif
