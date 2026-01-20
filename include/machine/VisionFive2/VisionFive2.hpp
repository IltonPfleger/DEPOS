#pragma once

#include "CacheController.hpp"
#include "Traits.hpp"

#include <architecture/rv/64/RV64.hpp>
#include <drivers/Driver.hpp>
#include <drivers/ethernet/DWMAC.hpp>
#include <drivers/uart/DW8250.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>

class VisionFive2 {

    class Clock : Driver {
        static constexpr unsigned long k_sys_crg_base = 0x13020000;
        static constexpr unsigned long k_aon_crg_base = 0x17000000;
        static constexpr unsigned long k_aon_syscon_base = 0x17010000;

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
            Reg32(k_aon_crg_base, GMAC5_0_CLK_TX) |=
                (1 << GMAC5_0_CLK_TX_CLK_MUX_SEL_SHIFT) & GMAC5_0_CLK_TX_CLK_MUX_SEL_MASK;
        }
    };

  public:
    using CPU = rv64::CPU;
    using Boot = rv64::Boot;
    using Ethernet = DWMAC<Traits<MemoryMap>::GMAC0>::Ethernet;
    using IO = DW8250<Traits<MemoryMap>::UART>;

    __attribute__((always_inline)) static inline void init() {
        Boot::probe();

        if (CPU::id() == Traits<CPUS>::BSP) {
            BSS::init();
            Clock::init();
        }

        CPU::barrier();
        Boot::init();
        IO::init();
    }
};
