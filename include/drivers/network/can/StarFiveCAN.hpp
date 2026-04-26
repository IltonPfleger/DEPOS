#pragma once

#include <Traits.hpp>
#include <architecture/IC.hpp>
#include <drivers/Driver.hpp>
#include <kernel/Alarm.hpp>
#include <network/NetworkDevice.hpp>
#include <network/can/CAN.hpp>
#include <shared/console/Console.hpp>
#include <shared/libraries/libc/string.h>
#include <shared/utility/Align.hpp>

namespace DEPOS {

template <typename Tag> class StarFiveCAN : public NetworkDevice {
    using Self     = StarFiveCAN<Tag>;
    using MyTraits = Traits<Self>;

    enum Registers {
        RBUF_ID   = 0x00,
        RBUF_CTL  = 0x04,
        RBUF_DATA = 0x08,
        TBUF_ID   = 0x50,
        TBUF_CTL  = 0x54,
        TBUF_DATA = 0x58,
        CFG_STAT  = 0xA0,
        TCMD      = 0xA1,
        RCTRL     = 0xA3,
        RTIE      = 0xA4,
        RTIF      = 0xA5,
        ERRINT    = 0xA6,
        LIMIT     = 0xA7,
        S_SEG_1   = 0xA8,
        F_SEG_1   = 0xAC,
        EALCAP    = 0xB0,
    };

    enum Masks {
        RESET             = 1 << 7,
        RIF               = 1 << 7,
        IDE               = 1 << 7,
        SACK              = 1 << 7,
        RTR               = 1 << 6,
        LBE               = 1 << 6,
        RFIF              = 1 << 5,
        LBI               = 1 << 5,
        RAFIF             = 1 << 4,
        RREL              = 1 << 4,
        TPE               = 1 << 4,
        TPIF              = 1 << 3,
        TSIF              = 1 << 2,
        EIF               = 1 << 1,
        DLC               = 0x0F,
        STBY              = 0x20,
        TBSEL             = 0x80,
        ALL_IRQ           = 0xFF,
        RX_NOT_EMPTY_MASK = 0x03,
        RX_IRQ            = RIF | RFIF | RAFIF,
    };

    static constexpr size_t MTU     = 8;
    static constexpr uintptr_t Base = MyTraits::Address;

    // Bit Timings: {baudrate, sample point, tq, prop, seg1, seg2, sjw, brp}
    static constexpr size_t BitTimings[][8] = {{495000, 875, 126, 6, 7, 2, 1, 5}};

  public:
    StarFiveCAN() {
        constexpr bool InternalLoopback = false;
        constexpr bool ExternalLoopback = true;

        IC::install(MyTraits::IRQs[0], onTrap);

        ioset8(Registers::CFG_STAT, Masks::RESET);

        baudrate(0);

        ioclear8(Registers::CFG_STAT, Masks::RESET);

        iowrite8(Registers::LIMIT, 0x40);
        ioset8(Registers::LIMIT, 0x0B);
        iowrite8(Registers::RTIE, Masks::ALL_IRQ);
        ioset8(Registers::ERRINT, 0x20);

        configure(InternalLoopback, ExternalLoopback);
    }

    virtual int send(const NetworkBuffer *nbuffer) override {
        auto *buffer = static_cast<const CAN::Buffer *>(nbuffer);

        uint8_t tcmd = ioread8(Registers::TCMD);
        tcmd &= ~Masks::TBSEL;
        tcmd &= ~Masks::STBY;
        iowrite8(Registers::TCMD, tcmd);

        reg32(Registers::TBUF_ID) = buffer->id();

        uint32_t ctl = (buffer->length() & Masks::DLC);
        if (buffer->isRemote()) ctl |= Masks::RTR;
        if (buffer->isExtended()) ctl |= Masks::IDE;
        reg32(Registers::TBUF_CTL) = ctl;

        uint32_t *data                  = buffer->data<uint32_t *>();
        reg32(Registers::TBUF_DATA)     = data[0];
        reg32(Registers::TBUF_DATA + 4) = data[1];

        ioset8(Registers::TCMD, Masks::TPE);
        return 0;
    }

    virtual const NetworkBuffer *receive() override {
        if (!(ioread8(Registers::RCTRL) & Masks::RX_NOT_EMPTY_MASK)) return nullptr;

        uint32_t id = reg32(Registers::RBUF_ID);
        uint8_t ctl = ioread8(Registers::RBUF_CTL);
        uint8_t dlc = (ctl & Masks::DLC) > MTU ? (ctl & Masks::DLC) : MTU;
        bool ide    = ctl & Masks::IDE;
        bool rtr    = ctl & Masks::RTR;

        auto *buffer = new CAN::Buffer(id, ide, rtr);
        buffer->length(dlc);

        if (!rtr) {
            uint32_t *data = buffer->data<uint32_t *>();
            data[0]        = reg32(Registers::RBUF_DATA);
            data[1]        = reg32(Registers::RBUF_DATA + 4);
        }

        ioset8(Registers::RCTRL, Masks::RREL);
        return buffer;
    }

    virtual void free(const NetworkBuffer *nbuffer) override { delete static_cast<const CAN::Buffer *>(nbuffer); }

  private:
    static void onTrap(size_t) {
        uint8_t isr = ioread8(Registers::RTIF);
        uint8_t eir = ioread8(Registers::ERRINT);

        if (!isr && !eir) return;

        if (isr & Masks::RX_IRQ) {
            iowrite8(Registers::RTIF, isr & Masks::RX_IRQ);
        }

        if (isr & (Masks::TPIF | Masks::TSIF)) {
            iowrite8(Registers::RTIF, isr & (Masks::TPIF | Masks::TSIF));
        }

        if ((isr & Masks::EIF) || (eir & 0x11)) {
            onError(eir);
        }
    }

    static void onError(uint8_t eir) {
        uint8_t koer    = ioread8(Registers::EALCAP) & 0xE0;
        const char *msg = "Unknown Error";

        switch (koer) {
        case 0x20:
            msg = "Bit Error";
            break;
        case 0x40:
            msg = "Form Error";
            break;
        case 0x60:
            msg = "Stuff Error";
            break;
        case 0x80:
            msg = "ACK Error";
            break;
        case 0xA0:
            msg = "CRC Error";
            break;
        }

        Console::println("CAN Error: ", msg);
        iowrite8(Registers::ERRINT, eir);
    }

    void configure(bool ilb, bool elb) {
        uint8_t status = ioread8(Registers::CFG_STAT) & ~(Masks::LBI | Masks::LBE);

        if (ilb) status |= Masks::LBI;
        if (elb) {
            status |= Masks::LBE;
            ioset8(Registers::RCTRL, Masks::SACK);
        } else {
            ioclear8(Registers::RCTRL, Masks::SACK);
        }
        iowrite8(Registers::CFG_STAT, status);
    }

    static void baudrate(size_t index) {
        auto b     = BitTimings[index];
        uint32_t s = ((b[4] + b[3] - 1) << 0) | ((b[5] - 1) << 8) | ((b[6] - 1) << 16) | ((b[7] - 1) << 24);
        reg32(Registers::S_SEG_1) = s;
    }

    static inline uint8_t ioread8(uintptr_t addr) {
        uintptr_t base = utility::Align::down(Base + addr, 4);
        uint8_t offset = (Base + addr) - base;
        return (reinterpret_cast<volatile uint32_t *>(base)[0] >> (offset * 8)) & 0xFF;
    }

    static inline void iowrite8(uintptr_t addr, uint8_t value) {
        uintptr_t base         = utility::Align::down(Base + addr, 4);
        uint8_t offset         = (Base + addr) - base;
        volatile uint32_t *ptr = reinterpret_cast<volatile uint32_t *>(base);

        uint32_t tmp = *ptr;
        tmp &= ~(0xFF << (offset * 8));
        tmp |= (static_cast<uint32_t>(value) << (offset * 8));
        *ptr = tmp;
    }

    static void ioset8(uintptr_t addr, uint8_t mask) { iowrite8(addr, ioread8(addr) | mask); }
    static void ioclear8(uintptr_t addr, uint8_t mask) { iowrite8(addr, ioread8(addr) & ~mask); }
    static volatile uint32_t &reg32(size_t offset) { return *reinterpret_cast<volatile uint32_t *>(Base + offset); }
};

} // namespace DEPOS
