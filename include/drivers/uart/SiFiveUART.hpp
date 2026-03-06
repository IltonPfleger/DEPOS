#pragma once

#include <abstractions/IC.hpp>
#include <drivers/Driver.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename Tag>
class SiFiveUART : public Driver, public Observed<const unsigned char *, size_t> {
    using MyTraits                         = Traits<SiFiveUART<Tag>>;
    static constexpr unsigned long Address = MyTraits::Address;

  private:
    SiFiveUART() = default;

    enum Registers { TXDATA = 0, TX_EMPTY_MASK = 1 << 31 };

    static void handler(unsigned int) {}

  public:
    static void init() {
        for (auto IRQ : MyTraits::IRQs)
            IC::bind(IRQ, handler);
    }

    static SiFiveUART *instance() {
        static SiFiveUART instance;
        return &instance;
    }

    void putc(char c) {
        while (Reg32(Address, TXDATA) & TX_EMPTY_MASK)
            ;
        Reg32(Address, TXDATA) = c;
    }
};

} // namespace DEPOS
