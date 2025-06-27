export module UART;

export struct UART {
    static constexpr int Clock    = 24000000;
    static constexpr int Baudrate = 115200;
    static constexpr int DIVISOR  = (Clock) / (Baudrate * 16);
    static volatile inline char* BaseAddr() { return reinterpret_cast<volatile char*>(0x10000000); }
    static volatile char& THR() { return *reinterpret_cast<volatile char*>(BaseAddr() + 0); };
    static volatile char& IER() { return *reinterpret_cast<volatile char*>(BaseAddr() + 1); };
    static volatile char& FCR() { return *reinterpret_cast<volatile char*>(BaseAddr() + 2); };
    static volatile char& LCR() { return *reinterpret_cast<volatile char*>(BaseAddr() + 3); };
    static volatile char& MCR() { return *reinterpret_cast<volatile char*>(BaseAddr() + 4); };
    static volatile char& LSR() { return *reinterpret_cast<volatile char*>(BaseAddr() + 5); };
    static volatile char& LSB() { return *reinterpret_cast<volatile char*>(BaseAddr() + 0); };  // Divisor Latch LSB
    static volatile char& MSB() { return *reinterpret_cast<volatile char*>(BaseAddr() + 1); };  // Divisor Latch MSB
    static constexpr char LSR_TX_EMPTY = (1 << 5);

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
