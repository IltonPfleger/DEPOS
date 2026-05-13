#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/init.hpp>
#include <drivers/Driver.hpp>
#include <drivers/cache/SiFiveU74L2CacheController.hpp>
#include <drivers/can/IPMSCANFD.hpp>
#include <drivers/ethernet/DWC_Ether_QoS.hpp>
#include <drivers/uart/UART16550.hpp>
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

    enum { GATE = 1 << 31 };

  public:
    enum class SystemClock {
        CLK_APB0                   = 0x30,
        JH7110_CAN0_CTRL_CLK_APB   = 0x1cc,
        JH7110_CAN0_CTRL_CLK_TIMER = 0x1d0,
        JH7110_CAN0_CTRL_CLK_CORE  = 0x1d4,
    };

    enum class SystemClockReset {
        RSTN_U0_CAN_CTRL_APB   = (3 << 16) | 15,
        RSTN_U0_CAN_CTRL_CORE  = (3 << 16) | 16,
        RSTN_U0_CAN_CTRL_TIMER = (3 << 16) | 17,
    };

    static void enable(SystemClock domain) { Reg32(k_sys_crg_base, static_cast<int>(domain)) |= GATE; }

    static void reset(SystemClockReset domain) {
        int integer = static_cast<int>(domain);
        int index   = (integer >> 16) & 0xFFFF;
        int bit     = 1 << (integer & 0xFFFF);

        uintptr_t assert = 0x2f8 + (4 * index);
        uintptr_t status = 0x308 + (4 * index);

        Reg32(k_sys_crg_base, assert) |= bit;

        while (Reg32(k_sys_crg_base, status) & bit)
            ;

        Reg32(k_sys_crg_base, assert) &= ~bit;

        while (!(Reg32(k_sys_crg_base, status) & bit))
            ;
    }

    static void divisor(SystemClock domain, unsigned int value) {
        Reg32(k_sys_crg_base, static_cast<int>(domain)) &= ~0xFFFFFF;
        Reg32(k_sys_crg_base, static_cast<int>(domain)) |= value;
    }
};

class VisionFive2 : Driver {
    // static constexpr unsigned long k_sys_crg_base    = 0x13020000;
    // static constexpr unsigned long k_aon_crg_base    = 0x17000000;
    // static constexpr unsigned long k_aon_syscon_base = 0x17010000;

    // enum {
    //     SYSCRG_GMAC0_GTX       = 0x1b0,
    //     SYSCRG_GMAC_PHY        = 0x1b8,
    //     SYSCRG_GMAC5_AHB       = 0x184,
    //     SYSCRG_GMAC5_AXI       = 0x188,
    //     SYSCRG_GMAC_SOURCE     = 0x18c,
    //     SYSCRG_SOFTWARE_RESET2 = 0x300,
    //     SYSCRG_RESET_STATUS2   = 0x310,
    //     AONCRG_GMAC0_AHB       = 0x8,
    //     AONCRG_GMAC0_RMII_RTX  = 0x10,
    //     AONCRG_GMAC0_AXI       = 0xc,
    //     AONCRG_GMAC0_TX        = 0x14,
    //     AONCRG_GMAC0_TX_INV    = 0x18,
    //     AONCRG_SOFTWARE_RESET  = 0x38,
    //     AONCRG_RESET_STATUS    = 0x3c,
    // };

    // enum {
    //     GATE     = 1 << 31,
    //     INVERTER = 1 << 30,
    // };

    // static void syscrg_init() {
    //     Reg32(k_sys_crg_base, SYSCRG_GMAC_PHY) |= GATE;
    //     Reg32(k_sys_crg_base, SYSCRG_GMAC_SOURCE) |= GATE;
    //     Reg32(k_sys_crg_base, SYSCRG_GMAC5_AHB) |= GATE;
    //     Reg32(k_sys_crg_base, SYSCRG_GMAC5_AXI) |= GATE;
    //     Reg32(k_sys_crg_base, SYSCRG_GMAC0_GTX) |= GATE;
    //     static constexpr auto RESET = (1 << 2) | (1 << 3);
    //     Reg32(k_sys_crg_base, SYSCRG_SOFTWARE_RESET2) |= RESET;
    //     while ((Reg32(k_sys_crg_base, SYSCRG_RESET_STATUS2) & RESET))
    //         ;
    //     Reg32(k_sys_crg_base, SYSCRG_SOFTWARE_RESET2) &= ~RESET;
    //     while (!(Reg32(k_sys_crg_base, SYSCRG_RESET_STATUS2) & RESET))
    //         ;
    // }

    // static void aoncrg_init() {
    //     Reg32(k_aon_crg_base, AONCRG_GMAC0_RMII_RTX) = 30;
    //     Reg32(k_aon_crg_base, AONCRG_GMAC0_TX) |= GATE;
    //     Reg32(k_aon_crg_base, AONCRG_GMAC0_TX) |= (1 << 24);
    //     Reg32(k_aon_crg_base, AONCRG_GMAC0_TX_INV) |= INVERTER;
    //     Reg32(k_aon_crg_base, AONCRG_GMAC0_AHB) |= GATE;
    //     Reg32(k_aon_crg_base, AONCRG_GMAC0_AXI) |= GATE;
    //     Reg32(k_aon_crg_base, AONCRG_SOFTWARE_RESET) |= 3;
    //     while ((Reg32(k_aon_crg_base, AONCRG_RESET_STATUS) & 3))
    //         ;
    //     Reg32(k_aon_crg_base, AONCRG_SOFTWARE_RESET) &= ~3;
    //     while (!(Reg32(k_aon_crg_base, AONCRG_RESET_STATUS) & 3))
    //         ;
    // }

  public:
    static void init() {
        riscv64::init();

        if (riscv64::CPU::id() == Traits<CPU>::BSP) {
            Clock::divisor(Clock::SystemClock::JH7110_CAN0_CTRL_CLK_CORE, 15);

            Clock::enable(Clock::SystemClock::JH7110_CAN0_CTRL_CLK_APB);
            Clock::enable(Clock::SystemClock::JH7110_CAN0_CTRL_CLK_TIMER);
            Clock::enable(Clock::SystemClock::JH7110_CAN0_CTRL_CLK_CORE);

            Clock::reset(Clock::SystemClockReset::RSTN_U0_CAN_CTRL_APB);
            Clock::reset(Clock::SystemClockReset::RSTN_U0_CAN_CTRL_CORE);
            Clock::reset(Clock::SystemClockReset::RSTN_U0_CAN_CTRL_TIMER);

            GPIO::map(GPIO::OutputSignal::GPO_SYS_IOMUX_U0_CAN_CTRL_TXD, 42);
            GPIO::map(GPIO::InputSignal::GPI_SYS_IOMUX_U0_CAN_CTRL_RXD, 43);
            GPIO::map(GPIO::OutputSignal::GPO_SYS_IOMUX_U0_CAN_CTRL_STB, 47);

            // syscrg_init();
            // aoncrg_init();
        }

        Meta::forEach(Traits<UART>::Devices{}, []<typename T>() { T::init(); });
        riscv64::CPU::barrier();
    }
};

} // namespace DEPOS
