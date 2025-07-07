struct SiFiveUART {
    static constexpr int Clock    = 31250000;
    static constexpr int Baudrate = 115200;
    static constexpr int DIVISOR  = Clock / Baudrate;

    static volatile inline unsigned int *BaseAddr() { return reinterpret_cast<volatile unsigned int *>(0x10010000); }
    static volatile unsigned int &TXDATA() { return *(BaseAddr() + 0); }
    static volatile unsigned int &RXDATA() { return *(BaseAddr() + 1); }
    static volatile unsigned int &TXCTRL() { return *(BaseAddr() + 2); }
    static volatile unsigned int &RXCTRL() { return *(BaseAddr() + 3); }
    static volatile unsigned int &IE() { return *(BaseAddr() + 4); }
    static volatile unsigned int &IP() { return *(BaseAddr() + 5); }
    static volatile unsigned int &DIV() { return *(BaseAddr() + 6); }

    static constexpr unsigned int TX_EMPTY_MASK = (1 << 31);
    static constexpr unsigned int RX_EMPTY_MASK = 0x80000000;

    static void init() {
        DIV()    = DIVISOR;
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
