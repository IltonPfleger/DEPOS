#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/init.hpp>
#include <machine/VisionFive2/ClockController.hpp>
#include <machine/VisionFive2/GPIO.hpp>

namespace DEPOS {

class VisionFive2 : Driver {
  public:
    static void init() {
        riscv64::init();
        if (riscv64::CPU::id() == Traits<CPU>::BSP) {
            /* ---***--- GMAC0 ---***--- */
            ClockController::enable(ClockController::SYSCRG_CLK_GMAC_PHY);
            ClockController::enable(ClockController::SYSCRG_CLK_GMAC0_GTX);
            ClockController::enable(ClockController::SYSCRG_CLK_GMAC_SOURCE);
            ClockController::enable(ClockController::SYSCRG_CLK_GMAC5_AXI64_AHB);
            ClockController::enable(ClockController::SYSCRG_CLK_GMAC5_AXI64_AXI);
            ClockController::reset(ClockController::SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_ARESETN_I);
            ClockController::reset(ClockController::SYSCRG_CLK_RSTN_U1_GMAC5_AXI64_HRESET_N);
            ClockController::divide(ClockController::AONCRG_CLK_GMAC0_RMII_RTX, 30);
            ClockController::enable(ClockController::AONCRG_CLK_GMAC0_TX);
            ClockController::invert(ClockController::AONCRG_CLK_GMAC5_AXI64_TX_INVERTER, true);
            ClockController::multiplex(ClockController::AONCRG_CLK_GMAC0_TX, 1);
            ClockController::enable(ClockController::AONCRG_CLK_GMAC0_AHB);
            ClockController::enable(ClockController::AONCRG_CLK_GMAC0_AXI);
            ClockController::reset(ClockController::AONCRG_CLK_RSTN_GMAC5_AXI64_AXI);
            ClockController::reset(ClockController::AONCRG_CLK_RSTN_GMAC5_AXI64_AHB);

            /* ---***--- CAN0 ---***--- */
            ClockController::divide(ClockController::SYSCRG_CLK_CAN0_CTRL_CORE, 15);
            ClockController::enable(ClockController::SYSCRG_CLK_CAN0_CTRL_APB);
            ClockController::enable(ClockController::SYSCRG_CLK_CAN0_CTRL_TIMER);
            ClockController::enable(ClockController::SYSCRG_CLK_CAN0_CTRL_CORE);
            ClockController::reset(ClockController::SYSCRG_CLK_RSTN_U0_CAN_CTRL_APB);
            ClockController::reset(ClockController::SYSCRG_CLK_RSTN_U0_CAN_CTRL_CORE);
            ClockController::reset(ClockController::SYSCRG_CLK_RSTN_U0_CAN_CTRL_TIMER);
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
