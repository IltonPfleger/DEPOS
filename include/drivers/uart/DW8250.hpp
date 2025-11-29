template <unsigned long A, unsigned long C, unsigned long B> struct DW8250 {
    // static constexpr unsigned long Divisor = (C / 16) / B;

    // static volatile inline unsigned char *BaseAddr() { return reinterpret_cast<volatile unsigned char *>(A); }

    static volatile unsigned char &LSR() { return *reinterpret_cast<unsigned char *>(A + 0x14); }
    static volatile unsigned char &TXD() { return *reinterpret_cast<unsigned char *>(A); }

    enum { TX_EMPTY = 1ULL << 5 };

    static void init() {}
    // IER() = 0x00;

    // LCR() |= DLAB_ENABLE;
    // DIV_LSB() = Divisor & 0xFF;
    // DIV_MSB() = (Divisor >> 8) & 0xFF;
    // LCR() &= ~DLAB_ENABLE;

    // unsigned char lcr = (db - 5); // 5,6,7,8 bits → 0..3
    // if (parity) {
    //     lcr |= PARITY_MASK;
    //     lcr |= (parity - 1) << 4; // odd=1, even=2
    // }
    // if (sb > 1)
    //     lcr |= STOP_BITS_MASK;

    // LCR() = lcr;

    // FCR() = 0xC7;

    // MCR() |= 0x0B;
    //}

    static void put(char c) {
        while (!(LSR() & TX_EMPTY))
            ;
        TXD() = c;
    }
};
