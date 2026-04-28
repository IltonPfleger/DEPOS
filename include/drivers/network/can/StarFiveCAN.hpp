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
  private:
    using Self   = StarFiveCAN<Tag>;
    using Traits = DEPOS::Traits<Self>;

    // ============================================================
    // Constants
    // ============================================================

    static constexpr uintptr_t Base    = Traits::Address;
    static constexpr size_t MaxPayload = 8;

    static constexpr bool DebugMode        = true;
    static constexpr bool InternalLoopback = false;
    static constexpr bool ExternalLoopback = false;

    // ============================================================
    // Registers
    // ============================================================

    enum class Reg : uintptr_t {
        RBUF_ID   = 0x00,
        RBUF_CTL  = 0x04,
        RBUF_DATA = 0x08,

        TBUF_ID   = 0x50,
        TBUF_CTL  = 0x54,
        TBUF_DATA = 0x58,

        STATUS  = 0xA0,
        TCMD    = 0xA1,
        RCTRL   = 0xA3,
        RTIE    = 0xA4,
        RTIF    = 0xA5,
        ERRINT  = 0xA6,
        LIMIT   = 0xA7,
        S_SEG_1 = 0xA8,
        F_SEG_1 = 0xAC,
        EALCAP  = 0xB0
    };

    // ============================================================
    // Bit Masks
    // ============================================================

    enum Mask : uint8_t {
        RESET = 0x80,

        // Error
        EIF  = 0x02,
        EPIE = 0x10,
        EPIF = 0x10,
        BEIF = 0x01,

        // RX
        RIE   = 0x02,
        RIF   = 0x80,
        RFIF  = 0x20,
        RAFIF = 0x10,
        RREL  = 0x10,
        RNE   = 0x03,

        // TX
        TPIF  = 0x08,
        TSIF  = 0x04,
        TPE   = 0x10,
        TBSEL = 0x80,

        // Frame
        IDE = 0x80,
        RTR = 0x40,
        DLC = 0x0F,

        // Modes
        SACK = 0x80,
        LBE  = 0x40,
        LBI  = 0x20,
        STBY = 0x20,

        // Status
        BUSOFF = 0x01
    };

    // ============================================================
    // Baudrate Table
    // ============================================================

    struct BitTiming {
        size_t baudrate;
        size_t t[6];
    };

    static constexpr BitTiming TimingTable[] = {
        {1000, {126, 2, 3, 2, 1, 5}}, {800, {126, 3, 4, 2, 1, 5}},   {500, {126, 6, 7, 2, 1, 5}},
        {250, {251, 6, 7, 2, 1, 10}}, {125, {1338, 1, 2, 2, 1, 53}}, {100, {833, 4, 5, 2, 1, 33}},
        {50, {1666, 4, 5, 2, 1, 66}}, {20, {3333, 6, 6, 2, 1, 132}}, {10, {6666, 6, 6, 2, 1, 264}}};

  public:
    // ============================================================
    // Constructor
    // ============================================================

    StarFiveCAN() {
        IC::install(Traits::IRQs[0], onTrap);

        set8(Reg::STATUS, RESET);

        configure(500, InternalLoopback, ExternalLoopback);

        clear8(Reg::STATUS, RESET);
        set8(Reg::ERRINT, EPIE);
    }

    // ============================================================
    // TX
    // ============================================================

    int send(const NetworkBuffer *raw) override {
        auto *frame = static_cast<const CAN::Buffer *>(raw);

        uint8_t cmd = read8(Reg::TCMD);
        cmd &= ~TBSEL;
        cmd &= ~STBY;
        write8(Reg::TCMD, cmd);

        reg32(Reg::TBUF_ID) = frame->id();

        uint32_t ctl = frame->length() & DLC;
        if (frame->isRemote()) ctl |= RTR;
        if (frame->isExtended()) ctl |= IDE;

        reg32(Reg::TBUF_CTL) = ctl;

        auto *data                                        = frame->data<uint32_t *>();
        reg32(Reg::TBUF_DATA)                             = data[0];
        reg32(static_cast<uintptr_t>(Reg::TBUF_DATA) + 4) = data[1];

        set8(Reg::TCMD, TPE);
        return 0;
    }

    // ============================================================
    // RX
    // ============================================================

    const NetworkBuffer *receive() override {
        if (!(read8(Reg::RCTRL) & RNE)) return nullptr;

        uint32_t id = reg32(Reg::RBUF_ID);
        uint8_t ctl = read8(Reg::RBUF_CTL);

        uint8_t dlc = ctl & DLC;
        if (dlc > MaxPayload) dlc = MaxPayload;

        bool ide = ctl & IDE;
        bool rtr = ctl & RTR;

        auto *frame = new CAN::Buffer(id, ide, rtr);
        frame->length(dlc);

        if (!rtr) {
            auto *data = frame->data<uint32_t *>();
            data[0]    = reg32(Reg::RBUF_DATA);
            data[1]    = reg32(static_cast<uintptr_t>(Reg::RBUF_DATA) + 4);
        }

        set8(Reg::RCTRL, RREL);
        return frame;
    }

    void free(const NetworkBuffer *raw) override { delete static_cast<const CAN::Buffer *>(raw); }

  private:
    // ============================================================
    // Handlers
    // ============================================================

    static void onTrap(size_t) {
        uint8_t flags = read8(Reg::RTIF);
        uint8_t err   = read8(Reg::ERRINT);

        if (!flags && !err) return;

        if ((flags & EIF) || (err & EPIF) || (err & BEIF)) onError(err);

        if (flags) set8(Reg::RTIF, flags);
    }

    static void onError(uint8_t err) {
        if constexpr (!DebugMode) return;

        uint8_t cause  = read8(Reg::EALCAP) & 0xE0;
        uint8_t status = read8(Reg::STATUS);

        const char *msg = "Unknown Error";

        switch (cause) {
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

        if (status & BUSOFF) msg = "Bus Off";

        Warn(msg);

        set8(Reg::ERRINT, err);
    }

    // ============================================================
    // Configuration
    // ============================================================

    void configure(size_t kbps, bool internalLB, bool externalLB) {
        for (const auto &cfg : TimingTable) {
            if (cfg.baudrate == kbps) {
                setBaudrate(cfg);
                break;
            }
        }

        if (internalLB) set8(Reg::STATUS, LBI);

        if (externalLB) {
            set8(Reg::STATUS, LBE);
            set8(Reg::RCTRL, SACK);
        }
    }

    static void setBaudrate(const BitTiming &cfg) {
        const auto &b = cfg.t;

        uint32_t value = ((b[1] + b[2] - 1) << 0) | ((b[3] - 1) << 8) | ((b[4] - 1) << 16) | ((b[5] - 1) << 24);

        reg32(Reg::S_SEG_1) = value;
    }

    // ============================================================
    // MMIO Helpers
    // ============================================================

    static uint8_t read8(Reg reg) {
        uintptr_t addr    = Base + static_cast<uintptr_t>(reg);
        uintptr_t aligned = utility::Align::down(addr, 4);
        uint8_t shift     = (addr - aligned) * 8;

        return (reinterpret_cast<volatile uint32_t *>(aligned)[0] >> shift) & 0xFF;
    }

    static void write8(Reg reg, uint8_t value) {
        uintptr_t addr    = Base + static_cast<uintptr_t>(reg);
        uintptr_t aligned = utility::Align::down(addr, 4);
        uint8_t shift     = (addr - aligned) * 8;

        auto *ptr = reinterpret_cast<volatile uint32_t *>(aligned);

        uint32_t tmp = *ptr;
        tmp &= ~(0xFFu << shift);
        tmp |= (static_cast<uint32_t>(value) << shift);

        *ptr = tmp;
    }

    static void set8(Reg reg, uint8_t mask) { write8(reg, read8(reg) | mask); }

    static void clear8(Reg reg, uint8_t mask) { write8(reg, read8(reg) & ~mask); }

    static volatile uint32_t &reg32(Reg reg) {
        return *reinterpret_cast<volatile uint32_t *>(Base + static_cast<uintptr_t>(reg));
    }

    static volatile uint32_t &reg32(uintptr_t offset) { return *reinterpret_cast<volatile uint32_t *>(Base + offset); }
};

} // namespace DEPOS
