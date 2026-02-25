#pragma once

#include <architecture/riscv64/init.hpp>

#include "Cache.hpp"
#include "Traits.hpp"

#include <drivers/Driver.hpp>
#include <drivers/uart/UART16550.hpp>
#include <memory/Memory.hpp>

#include <drivers/ethernet/DWC_Ether_QoS.hpp>

class VisionFive2 : Driver {
    static constexpr unsigned long k_sys_crg_base = 0x13020000;
    static constexpr unsigned long k_aon_crg_base = 0x17000000;
    static constexpr unsigned long k_aon_syscon_base = 0x17010000;

    enum {
        SYSCRG_GMAC0_GTX = 0x1b0,
        SYSCRG_GMAC_PHY = 0x1b8,
        SYSCRG_GMAC5_AHB = 0x184,
        SYSCRG_GMAC5_AXI = 0x188,
        SYSCRG_GMAC_SOURCE = 0x18c,
        SYSCRG_SOFTWARE_RESET2 = 0x310,
        AONCRG_GMAC0_AHB = 0x8,
        AONCRG_GMAC0_RMII_RTX = 0x10,
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
    }

    static void aoncrg_init() {
        Reg32(k_aon_crg_base, AONCRG_GMAC0_RMII_RTX) = 30;
        Reg32(k_aon_crg_base, AONCRG_GMAC0_TX) |= GATE;
        Reg32(k_aon_crg_base, AONCRG_GMAC0_TX) |= (1 << 24);
        Reg32(k_aon_crg_base, AONCRG_GMAC0_TX_INV) |= INVERTER;
        Reg32(k_aon_crg_base, AONCRG_GMAC0_AHB) |= GATE;
        Reg32(k_aon_crg_base, AONCRG_GMAC0_AXI) |= GATE;
        Reg32(k_aon_crg_base, AONCRG_SOFTWARE_RESET) |= 3;
        Reg32(k_aon_crg_base, AONCRG_SOFTWARE_RESET) &= ~3;
    }

  public:
    static void init() {
        if (riscv64::CPU::id() == Traits<CPU>::BSP) {
            syscrg_init();
            aoncrg_init();
        }

        riscv64::CPU::mb();

        riscv64::CPU::barrier();

        riscv64::init();

        riscv64::CPU::mb();

        Meta::ForEachTypeList(Traits<UART>::Devices{}, []<typename T>() { T::init(); });

        riscv64::CPU::mb();
    }
};
