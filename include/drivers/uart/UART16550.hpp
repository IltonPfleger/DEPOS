#pragma once

#include <abstractions/IC.hpp>
#include <drivers/Driver.hpp>
#include <utils/Debug.hpp>
#include <utils/Observer.hpp>

template <unsigned long Address, unsigned long BaudDivisor, unsigned long RxIRQ>
class UART16550 : public Driver, public Observed<unsigned char *, size_t> {
  private:
    UART16550() {
        Reg8(Address, IER) = 0x00;
        Reg8(Address, LCR) = LCR_DLAB;
        Reg8(Address, DLL) = (uint8_t)(BaudDivisor & 0xFF);
        Reg8(Address, DLM) = (uint8_t)((BaudDivisor >> 8) & 0xFF);
        Reg8(Address, LCR) = LCR_8N1;
        Reg8(Address, FCR) = FCR_ENABLE | FCR_CLEAR;
        Reg8(Address, IER) = IER_RX;
        Reg8(Address, MCR) = 0x0B;
    }

    enum Registers {
        RBR = 0,
        THR = 0,
        IER = 1,
        IIR = 2,
        FCR = 2,
        LCR = 3,
        MCR = 4,
        LSR = 5,
        MSR = 6,
        SCR = 7,
        DLL = 0,
        DLM = 1
    };

    enum Bits {
        LCR_DLAB = 1 << 7,
        LCR_8N1 = 0x03,
        FCR_ENABLE = 0x01,
        FCR_CLEAR = 0x06,
        IER_RX = 0x01,
        LSR_RX_READY = 1 << 0,
        LSR_TX_EMPTY = 1 << 5,
    };

    static void handler(unsigned int) {
        unsigned char buffer[16];
        unsigned int i = 0;
        while (Reg8(Address, LSR) & LSR_RX_READY && i < sizeof(buffer)) {
            unsigned char c = Reg8(Address, RBR);
            buffer[i] = c;
            i++;
        }
        instance()->notify(buffer, i);
    }

  public:
    static void init() { IC::bind(RxIRQ, handler); }

    static UART16550 *instance() {
        static UART16550 _;
        return &_;
    }

    void putc(char c) {
        while ((Reg8(Address, LSR) & LSR_TX_EMPTY) == 0)
            ;
        Reg8(Address, THR) = c;
    }

    char getc() {
        while ((Reg8(Address, LSR) & LSR_RX_READY) == 0)
            ;
        return Reg8(Address, RBR);
    }

    void write(const char *s) {
        while (s && *s)
            putc(*s++);
    }
};
