#pragma once

#include <architecture/IC.hpp>
#include <drivers/Driver.hpp>
#include <utility/Observer.hpp>

namespace DEPOS {

template <typename Tag> class SiFiveUART : public Driver, public Observed<const unsigned char *, size_t> {
    using MyTraits                         = Traits<SiFiveUART<Tag>>;
    static constexpr unsigned long Address = MyTraits::Address;

    enum {
        TXDATA = 0,
        RXDATA = 0x04,
        TXCTRL = 0x08,
        RXCTRL = 0x0c,
        IE     = 0x10,
        DIV    = 0x18,
    };

    enum { RXEN = 0x1, TXEN = 0x1, RXIRQ = 1 << 1, RXEMPTY = 1 << 31, TXFULL = 1 << 31 };

  private:
    SiFiveUART() {
        Reg32(Address, DIV)    = (MyTraits::Clock / MyTraits::BaudRate) - 1;
        Reg32(Address, RXCTRL) = RXEN;
        Reg32(Address, TXCTRL) = TXEN;
        Reg32(Address, IE) |= RXIRQ;
    }

    static void handler(size_t) {
        unsigned char buffer[8];
        unsigned int i = 0;
        while (i < sizeof(buffer)) {
            int c = Reg32(Address, RXDATA);
            if (c & RXEMPTY) break;
            buffer[i] = c & 0xFF;
            i++;
        }
        if (i > 0) instance()->notify(buffer, i);
    }

  public:
    static void init() {
        for (auto IRQ : MyTraits::IRQs)
            IC::install(IRQ, handler);
    }

    static SiFiveUART *instance() {
        static SiFiveUART instance;
        return &instance;
    }

    void putc(char c) {
        while (Reg32(Address, TXDATA) & TXFULL)
            ;
        Reg32(Address, TXDATA) = c;
    }
};

} // namespace DEPOS
