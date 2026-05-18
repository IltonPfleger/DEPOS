#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/init.hpp>
#include <machine/VisionFive2/ClockManager.hpp>
#include <machine/VisionFive2/GPIO.hpp>

namespace DEPOS {

class VisionFive2 : Driver {
  public:
    static void init() {
        riscv64::init();

        if (riscv64::CPU::id() == Traits<CPU>::BSP) {
            /* ---***--- GMAC0 ---***--- */
            ClockManager::enable(ClockManager::SYSCRG_CLK_GMAC_PHY);
            ClockManager::enable(ClockManager::SYSCRG_CLK_GMAC0_GTX);
            ClockManager::enable(ClockManager::SYSCRG_CLK_GMAC_SOURCE);
            ClockManager::enable(ClockManager::SYSCRG_CLK_GMAC5_AXI64_AHB);
            ClockManager::enable(ClockManager::SYSCRG_CLK_GMAC5_AXI64_AXI);

            ClockManager::reset(ClockManager::SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_ARESETN_I);
            ClockManager::reset(ClockManager::SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_HRESET_N);

            ClockManager::divide(ClockManager::AONCRG_CLK_GMAC0_RMII_RTX, 30);

            ClockManager::enable(ClockManager::AONCRG_CLK_GMAC0_TX);

            ClockManager::invert(ClockManager::AONCRG_CLK_GMAC5_AXI64_TX_INVERTER, true);

            ClockManager::multiplex(ClockManager::AONCRG_CLK_GMAC0_TX, 1);

            ClockManager::enable(ClockManager::AONCRG_CLK_GMAC0_AHB);
            ClockManager::enable(ClockManager::AONCRG_CLK_GMAC0_AXI);

            ClockManager::reset(ClockManager::AONCRG_CLK_RSTN_GMAC5_AXI64_AXI);
            ClockManager::reset(ClockManager::AONCRG_CLK_RSTN_GMAC5_AXI64_AHB);

            /* ---***--- CAN0 ---***--- */
            ClockManager::divide(ClockManager::SYSCRG_CLK_CAN0_CTRL_CORE, 15);
            ClockManager::enable(ClockManager::SYSCRG_CLK_CAN0_CTRL_APB);
            ClockManager::enable(ClockManager::SYSCRG_CLK_CAN0_CTRL_TIMER);
            ClockManager::enable(ClockManager::SYSCRG_CLK_CAN0_CTRL_CORE);

            ClockManager::reset(ClockManager::SYSCRG_CLK_RSTN_U0_CAN_CTRL_APB);
            ClockManager::reset(ClockManager::SYSCRG_CLK_RSTN_U0_CAN_CTRL_CORE);
            ClockManager::reset(ClockManager::SYSCRG_CLK_RSTN_U0_CAN_CTRL_TIMER);
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
