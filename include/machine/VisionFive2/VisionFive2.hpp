#pragma once

#include <architecture/riscv64/init.hpp>

#include "CacheController.hpp"
#include "Traits.hpp"

#include <drivers/Driver.hpp>
#include <drivers/uart/UART16550.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>

#include <drivers/ethernet/DWC_Ether_QoS.hpp>

class VisionFive2 : Driver {
    static constexpr unsigned long k_sys_crg_base = 0x13020000;
    static constexpr unsigned long k_aon_crg_base = 0x17000000;
    static constexpr unsigned long k_aon_syscon_base = 0x17010000;

    class Clock : Driver {
        enum Bit {
            CLOCK_ENABLE = 1 << 31,
            TIMER_RSTN_APB = 1 << 21,
            TIMER_RSTN_TIMER0 = 1 << 22,
            TIMER_RSTN_TIMER1 = 1 << 23,
            TIMER_RSTN_TIMER2 = 1 << 24,
            TIMER_RSTN_TIMER3 = 1 << 25,

        };

        enum Register {
            TIMER_CLK_APB_SHIFT = 0x1F0U,
            TIMER_CLK_TIMER0_SHIFT = 0x1F4U,
            TIMER_CLK_TIMER1_SHIFT = 0x1F8U,
            TIMER_CLK_TIMER2_SHIFT = 0x1FCU,
            TIMER_CLK_TIMER3_SHIFT = 0x200U,
            SYS_CRG_RESET_ASSERT2 = 0x300U,
            SYS_CRG_RESET_ASSERT3 = 0x304U,
            AON_CRG_RESET_ASSERT = 0x38U,
        };

        enum GMAC {
            GMAC5_0_CLK_TX_CLK_MUX_SEL_SHIFT = 24,
            GMAC5_0_CLK_TX = 0x14,
            GMAC5_0_CLK_TX_CLK_MUX_SEL_MASK = 0x1000000,
        };

        static void enable() {
            for (int offset = 0x184; offset <= 0x1BC; offset += 4) {
                Reg32(k_sys_crg_base, offset) |= CLOCK_ENABLE;
            }
            for (int offset = 0x8; offset <= 0x20; offset += 4) {
                Reg32(k_aon_crg_base, offset) |= CLOCK_ENABLE;
            }

            Reg32(k_sys_crg_base, TIMER_CLK_APB_SHIFT) |= CLOCK_ENABLE;
            Reg32(k_sys_crg_base, TIMER_CLK_TIMER0_SHIFT) |= CLOCK_ENABLE;
            Reg32(k_sys_crg_base, TIMER_CLK_TIMER1_SHIFT) |= CLOCK_ENABLE;
            Reg32(k_sys_crg_base, TIMER_CLK_TIMER2_SHIFT) |= CLOCK_ENABLE;
            Reg32(k_sys_crg_base, TIMER_CLK_TIMER3_SHIFT) |= CLOCK_ENABLE;
        }

      public:
        static void reset() {
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) |= TIMER_RSTN_APB;
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) &= ~TIMER_RSTN_APB;

            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) |= TIMER_RSTN_TIMER0;
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) &= ~TIMER_RSTN_TIMER0;

            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) |= TIMER_RSTN_TIMER1;
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) &= ~TIMER_RSTN_TIMER1;

            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) |= TIMER_RSTN_TIMER2;
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) &= ~TIMER_RSTN_TIMER2;

            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) |= TIMER_RSTN_TIMER3;
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT3) &= ~TIMER_RSTN_TIMER3;

            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT2) |= 0xc;
            Reg32(k_sys_crg_base, SYS_CRG_RESET_ASSERT2) &= ~0xc;

            Reg32(k_aon_crg_base, AON_CRG_RESET_ASSERT) |= 0x3;
            Reg32(k_aon_crg_base, AON_CRG_RESET_ASSERT) &= ~0x3;
        }

        static void init() {
            enable();
            reset();

            Reg32(k_aon_crg_base, GMAC5_0_CLK_TX) &= ~GMAC5_0_CLK_TX_CLK_MUX_SEL_MASK;
            Reg32(k_aon_crg_base, GMAC5_0_CLK_TX) |= (1 << GMAC5_0_CLK_TX_CLK_MUX_SEL_SHIFT) & GMAC5_0_CLK_TX_CLK_MUX_SEL_MASK;
        }
    };

    enum {
        SYSCRG_GMAC0_GTX = 0x1b0,
        SYSCRG_GMAC_PHY = 0x1b8,
        SYSCRG_GMAC5_AHB = 0x184,
        SYSCRG_GMAC5_AXI = 0x188,
        SYSCRG_GMAC_SOURCE = 0x18c,
        SYSCRG_SOFTWARE_RESET2 = 0x310,
        AONCRG_GMAC0_AHB = 0x8,
        AONCRG_GMAC0_AXI = 0xc,
        AONCRG_GMAC0_TX = 0x14,
        AONCRG_GMAC0_TX_INV = 0x18,
        AONCRG_SOFTWARE_RESET = 0x38,
        AONCRG_RESET_STATUS = 0x3c,
    };

    enum {
        GATE = 1 << 31,
        INVERTER = 1 << 30,
    };

    static void syscrg_init() {
        Reg32(k_sys_crg_base, SYSCRG_GMAC_PHY) |= GATE;
        Reg32(k_sys_crg_base, SYSCRG_GMAC_SOURCE) |= GATE;
        Reg32(k_sys_crg_base, SYSCRG_GMAC5_AHB) |= GATE;
        Reg32(k_sys_crg_base, SYSCRG_GMAC5_AXI) |= GATE;
        Reg32(k_sys_crg_base, SYSCRG_GMAC0_GTX) |= GATE;
        Reg32(k_sys_crg_base, SYSCRG_SOFTWARE_RESET2) |= (1 << 2) | (1 << 3);
        Reg32(k_sys_crg_base, SYSCRG_SOFTWARE_RESET2) &= ~((1 << 2) | (1 << 3));
        // Console::cout << Console::endl << Console::hex << Reg32(k_sys_crg_base, 0x14) << Console::endl;
    }

    static void aoncrg_init() {
        Reg32(k_aon_crg_base, AONCRG_GMAC0_TX) |= GATE;
        // Reg32(k_aon_crg_base, AONCRG_GMAC0_TX) &= ~(1 << 24);

        Reg32(k_aon_crg_base, AONCRG_GMAC0_TX_INV) |= INVERTER;
        Reg32(k_aon_crg_base, AONCRG_GMAC0_AHB) |= GATE;
        Reg32(k_aon_crg_base, AONCRG_GMAC0_AXI) |= GATE;
        Reg32(k_aon_crg_base, AONCRG_SOFTWARE_RESET) |= 3;
        Reg32(k_aon_crg_base, AONCRG_SOFTWARE_RESET) &= ~3;
    }

  public:
    static void init() {
        if (riscv64::CPU::id() == Traits<CPU>::BSP) {
            Clock::init();
            aoncrg_init();
            syscrg_init();
        }

        riscv64::CPU::barrier();

        riscv64::init();
    }
};
