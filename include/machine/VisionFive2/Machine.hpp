#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/init.hpp>
#include <drivers/Driver.hpp>
#include <memory/Memory.hpp>

namespace DEPOS {

class GPIO : Driver {
    static const uintptr_t k_sys_iomux_base = 0x13040000;

    enum {
        DOEN_OFFSET = 0,
        DOUT_OFFSET = 0x40,
        DIN_OFFSET  = 0x80,

        DOUT_MASK = 0x7f,
        DOEN_MASK = 0x3f,
        DIN_MASK  = 0xff,
    };

  public:
    enum class OutputSignal {
        GPO_SYS_IOMUX_U0_CAN_CTRL_TXD = 0x6,
        GPO_SYS_IOMUX_U0_CAN_CTRL_STB = 0x3,
    };

    enum class InputSignal {
        GPI_SYS_IOMUX_U0_CAN_CTRL_RXD = 0x1,
    };

    static void map(OutputSignal signal, unsigned int pin) {
        auto offset = 4 * (pin / 4);
        auto shift  = 8 * (pin % 4);
        auto dout   = static_cast<int>(signal);

        Reg32(k_sys_iomux_base, DOUT_OFFSET + offset) &= ~(DOUT_MASK << shift);
        Reg32(k_sys_iomux_base, DOUT_OFFSET + offset) |= dout << shift;

        Reg32(k_sys_iomux_base, DOEN_OFFSET + offset) &= ~(DOEN_MASK << shift);
    }

    static void map(InputSignal signal, unsigned int pin) {
        auto output_offset = 4 * (pin / 4);
        auto output_shift  = 8 * (pin % 4);

        auto integer      = static_cast<unsigned int>(signal);
        auto input_offset = 4 * (integer / 4);
        auto input_shift  = 8 * (integer % 4);

        Reg32(k_sys_iomux_base, DOUT_OFFSET + output_offset) &= ~(DOUT_MASK << output_shift);
        Reg32(k_sys_iomux_base, DOEN_OFFSET + output_offset) &= ~(DOEN_MASK << output_shift);
        Reg32(k_sys_iomux_base, DOEN_OFFSET + output_offset) |= (1 << output_shift);

        Reg32(k_sys_iomux_base, DIN_OFFSET + input_offset) &= ~(DIN_MASK << input_shift);
        Reg32(k_sys_iomux_base, DIN_OFFSET + input_offset) |= (pin + 2) << input_shift;
    }
};

class Clock : Driver {
    static const uintptr_t k_sys_crg_base    = 0x13020000;
    static const uintptr_t k_aon_crg_base    = 0x17000000;
    static const uintptr_t k_aon_syscon_base = 0x17010000;

    enum { SYSCRG = 0LL << 63, AONCRG = 1LL << 63, GATE = 1 << 31 };

  public:
    enum {
        SYSCRG_CLK_GMAC_PHY                = SYSCRG | 0x1b8,
        SYSCRG_CLK_GMAC0_GTX               = SYSCRG | 0x1b0,
        SYSCRG_CLK_GMAC_SOURCE             = SYSCRG | 0x18c,
        SYSCRG_CLK_GMAC5_AXI64_RX_INVERTER = SYSCRG | 0x1a0,
        SYSCRG_CLK_GMAC5_AXI64_TX_INVERTER = SYSCRG | 0x1a8,
        SYSCRG_CLK_GMAC5_AXI64_AHB         = SYSCRG | 0x184,
        SYSCRG_CLK_GMAC5_AXI64_AXI         = SYSCRG | 0x188,

        SYSCRG_CLK_CAN0_CTRL_APB   = SYSCRG | 0x1cc,
        SYSCRG_CLK_CAN0_CTRL_TIMER = SYSCRG | 0x1d0,
        SYSCRG_CLK_CAN0_CTRL_CORE  = SYSCRG | 0x1d4,

        SYSCRG_CLK_RSTN_U0_CAN_CTRL_APB          = SYSCRG | (3 << 16) | 15,
        SYSCRG_CLK_RSTN_U0_CAN_CTRL_CORE         = SYSCRG | (3 << 16) | 16,
        SYSCRG_CLK_RSTN_U0_CAN_CTRL_TIMER        = SYSCRG | (3 << 16) | 17,
        SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_ARESETN_I = SYSCRG | (2 << 16) | 2,
        SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_HRESET_N  = SYSCRG | (2 << 16) | 3,

        AONCRG_CLK_GMAC0_AHB               = AONCRG | 0x8,
        AONCRG_CLK_GMAC0_RMII_RTX          = AONCRG | 0x10,
        AONCRG_CLK_GMAC0_AXI               = AONCRG | 0xc,
        AONCRG_CLK_GMAC0_TX                = AONCRG | 0x14,
        AONCRG_CLK_GMAC5_AXI64_TX_INVERTER = AONCRG | 0x18,

        AONCRG_CLK_RSTN_GMAC5_AXI64_AXI = AONCRG | (0 << 16) | 0,
        AONCRG_CLK_RSTN_GMAC5_AXI64_AHB = AONCRG | (0 << 16) | 1,
    };

    static void enable(uint64_t id) {
        bool aon = id & AONCRG;
        id &= ~AONCRG;
        if (aon) {
            Reg32(k_aon_crg_base, id) |= GATE;
        } else {
            Reg32(k_sys_crg_base, id) |= GATE;
        }
    }

    static void divide(uint64_t id, uint32_t value) {
        bool aon = id & AONCRG;
        uintptr_t address;
        id &= ~AONCRG;
        if (aon) {
            address = k_aon_crg_base;
        } else {
            address = k_sys_crg_base;
        }
        Reg32(address, id) &= ~0xFFFFFF;
        Reg32(address, id) |= value;
    }

    static void invert(uintptr_t id, bool value) {
        bool aon = id & AONCRG;
        uintptr_t address;
        id &= ~AONCRG;
        if (aon) {
            address = k_aon_crg_base;
        } else {
            address = k_sys_crg_base;
        }
        Reg32(address, id) &= ~(1 << 30);
        Reg32(address, id) |= value << 30;
    }

    static void multiplex(uintptr_t id, unsigned int value) {
        bool aon = id & AONCRG;
        uintptr_t address;
        id &= ~AONCRG;
        if (aon) {
            address = k_aon_crg_base;
        } else {
            address = k_sys_crg_base;
        }
        Reg32(address, id) &= ~(0x3F << 24);
        Reg32(address, id) |= (value & 0x3F) << 24;
    }

    static void reset(uint64_t id) {
        bool aon  = id & AONCRG;
        int index = (id >> 16) & 0xFFFF;
        int bit   = 1 << (id & 0xFFFF);
        uintptr_t address;
        uintptr_t assert;
        uintptr_t status;

        id &= ~AONCRG;
        if (aon) {
            address = k_aon_crg_base;
            assert  = 0x38 + (4 * index);
            status  = 0x3c + (4 * index);
        } else {
            address = k_sys_crg_base;
            assert  = 0x2f8 + (4 * index);
            status  = 0x308 + (4 * index);
        }

        Reg32(address, assert) |= bit;

        while (Reg32(address, status) & bit)
            ;

        Reg32(address, assert) &= ~bit;

        while (!(Reg32(address, status) & bit))
            ;
    }
};

class VisionFive2 : Driver {
  public:
    static void init() {
        riscv64::init();

        if (riscv64::CPU::id() == Traits<CPU>::BSP) {
            /* ---***--- GMAC0 ---***--- */
            Clock::enable(Clock::SYSCRG_CLK_GMAC_PHY);
            Clock::enable(Clock::SYSCRG_CLK_GMAC0_GTX);
            Clock::enable(Clock::SYSCRG_CLK_GMAC_SOURCE);
            Clock::enable(Clock::SYSCRG_CLK_GMAC5_AXI64_AHB);
            Clock::enable(Clock::SYSCRG_CLK_GMAC5_AXI64_AXI);

            Clock::reset(Clock::SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_ARESETN_I);
            Clock::reset(Clock::SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_HRESET_N);

            Clock::divide(Clock::AONCRG_CLK_GMAC0_RMII_RTX, 30);

            Clock::enable(Clock::AONCRG_CLK_GMAC0_TX);

            Clock::invert(Clock::AONCRG_CLK_GMAC5_AXI64_TX_INVERTER, true);

            Clock::multiplex(Clock::AONCRG_CLK_GMAC0_TX, 1);

            Clock::enable(Clock::AONCRG_CLK_GMAC0_AHB);
            Clock::enable(Clock::AONCRG_CLK_GMAC0_AXI);

            Clock::reset(Clock::AONCRG_CLK_RSTN_GMAC5_AXI64_AXI);
            Clock::reset(Clock::AONCRG_CLK_RSTN_GMAC5_AXI64_AHB);

            /* ---***--- CAN0 ---***--- */
            Clock::divide(Clock::SYSCRG_CLK_CAN0_CTRL_CORE, 15);
            Clock::enable(Clock::SYSCRG_CLK_CAN0_CTRL_APB);
            Clock::enable(Clock::SYSCRG_CLK_CAN0_CTRL_TIMER);
            Clock::enable(Clock::SYSCRG_CLK_CAN0_CTRL_CORE);

            Clock::reset(Clock::SYSCRG_CLK_RSTN_U0_CAN_CTRL_APB);
            Clock::reset(Clock::SYSCRG_CLK_RSTN_U0_CAN_CTRL_CORE);
            Clock::reset(Clock::SYSCRG_CLK_RSTN_U0_CAN_CTRL_TIMER);
            GPIO::map(GPIO::OutputSignal::GPO_SYS_IOMUX_U0_CAN_CTRL_TXD, 42);
            GPIO::map(GPIO::InputSignal::GPI_SYS_IOMUX_U0_CAN_CTRL_RXD, 43);
            GPIO::map(GPIO::OutputSignal::GPO_SYS_IOMUX_U0_CAN_CTRL_STB, 47);
        }

        Meta::forEach(Traits<UART>::Devices{}, []<typename T>() { T::init(); });
        riscv64::CPU::barrier();
    }

    static void shutdown() { CPU::halt(); }
};

} // namespace DEPOS

#include <drivers/cache/SiFiveU74L2CacheController.hpp>
#include <drivers/can/IPMSCANFD.hpp>
#include <drivers/ethernet/DWC_Ether_QoS.hpp>
#include <drivers/uart/UART16550.hpp>
