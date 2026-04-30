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

template <typename Tag> class IPMS_CANFD : public NetworkDevice {
  private:
    using Traits = DEPOS::Traits<Tag>;

    // ============================================================
    // Constants
    // ============================================================
    static constexpr uintptr_t Base    = Traits::Address;
    static constexpr size_t MaxPayload = 8;

    static constexpr bool DebugMode        = true;
    static constexpr bool InternalLoopback = false;
    static constexpr bool ExternalLoopback = false;

    // ============================================================
    // Registeristers
    // ============================================================
    enum class Register : uintptr_t {
        RBUF_ID    = 0x00,
        RBUF_CTL   = 0x04,
        RBUF_DATA1 = 0x08,
        RBUF_DATA2 = 0x0C,

        TBUF_ID    = 0x50,
        TBUF_CTL   = 0x54,
        TBUF_DATA1 = 0x58,
        TBUF_DATA2 = 0x5C,

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
    IPMS_CANFD() {
        IC::install(Traits::IRQs[0], onTrap);

        set8(Register::STATUS, RESET);

        configure(500, InternalLoopback, ExternalLoopback);

        clear8(Register::STATUS, RESET);
        set8(Register::ERRINT, EPIE);
    }

    // ============================================================
    // TX
    // ============================================================
    int send(const NetworkBuffer *raw) override {
        auto *frame = static_cast<const CAN::Buffer *>(raw);

        uint8_t cmd = read8(Register::TCMD);
        cmd &= ~TBSEL;
        cmd &= ~STBY;
        write8(Register::TCMD, cmd);

        reg32(Register::TBUF_ID) = frame->id();

        uint32_t ctl = frame->length() & DLC;
        if (frame->isRemote()) ctl |= RTR;
        if (frame->isExtended()) ctl |= IDE;

        reg32(Register::TBUF_CTL) = ctl;

        auto *data                  = frame->data<uint32_t *>();
        reg32(Register::TBUF_DATA1) = data[0];
        reg32(Register::TBUF_DATA2) = data[1];

        set8(Register::TCMD, TPE);
        return 0;
    }

    // ============================================================
    // RX
    // ============================================================
    const NetworkBuffer *receive() override {
        if (!(read8(Register::RCTRL) & RNE)) return nullptr;

        uint32_t id = reg32(Register::RBUF_ID);
        uint8_t ctl = read8(Register::RBUF_CTL);

        uint8_t dlc = ctl & DLC;
        if (dlc > MaxPayload) dlc = MaxPayload;

        bool ide = ctl & IDE;
        bool rtr = ctl & RTR;

        auto *frame = new CAN::Buffer(id, ide, rtr);
        frame->length(dlc);

        if (!rtr) {
            auto *data = frame->data<uint32_t *>();
            data[0]    = reg32(Register::RBUF_DATA1);
            data[1]    = reg32(Register::RBUF_DATA2);
        }

        set8(Register::RCTRL, RREL);
        return frame;
    }

    void free(const NetworkBuffer *raw) override { delete static_cast<const CAN::Buffer *>(raw); }

  private:
    // ============================================================
    // Handlers
    // ============================================================
    static void onTrap(size_t) {
        uint8_t flags = read8(Register::RTIF);
        uint8_t err   = read8(Register::ERRINT);

        if (!flags && !err) return;

        if ((flags & EIF) || (err & EPIF) || (err & BEIF)) onError(err);

        if (flags) set8(Register::RTIF, flags);
    }

    static void onError(uint8_t err) {
        if constexpr (!DebugMode) return;

        uint8_t cause  = read8(Register::EALCAP) & 0xE0;
        uint8_t status = read8(Register::STATUS);

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

        set8(Register::ERRINT, err);
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

        if (internalLB) set8(Register::STATUS, LBI);

        if (externalLB) {
            set8(Register::STATUS, LBE);
            set8(Register::RCTRL, SACK);
        }
    }

    static void setBaudrate(const BitTiming &cfg) {
        const auto &b = cfg.t;

        uint32_t value = ((b[1] + b[2] - 1) << 0) | ((b[3] - 1) << 8) | ((b[4] - 1) << 16) | ((b[5] - 1) << 24);

        reg32(Register::S_SEG_1) = value;
    }

    // ============================================================
    // MMIO Helpers
    // ============================================================
    static uint8_t read8(Register reg) {
        uintptr_t addr    = Base + static_cast<uintptr_t>(reg);
        uintptr_t aligned = utility::Align::down(addr, 4);
        uint8_t shift     = (addr - aligned) * 8;

        return (reinterpret_cast<volatile uint32_t *>(aligned)[0] >> shift) & 0xFF;
    }

    static void write8(Register reg, uint8_t value) {
        uintptr_t addr    = Base + static_cast<uintptr_t>(reg);
        uintptr_t aligned = utility::Align::down(addr, 4);
        uint8_t shift     = (addr - aligned) * 8;

        auto *ptr = reinterpret_cast<volatile uint32_t *>(aligned);

        uint32_t tmp = *ptr;
        tmp &= ~(0xFFu << shift);
        tmp |= (static_cast<uint32_t>(value) << shift);

        *ptr = tmp;
    }

    static void set8(Register reg, uint8_t mask) { write8(reg, read8(reg) | mask); }

    static void clear8(Register reg, uint8_t mask) { write8(reg, read8(reg) & ~mask); }

    static volatile uint32_t &reg32(Register reg) {
        return *reinterpret_cast<volatile uint32_t *>(Base + static_cast<uintptr_t>(reg));
    }
};

} // namespace DEPOS
