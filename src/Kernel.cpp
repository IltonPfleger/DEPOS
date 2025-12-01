#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>
#include <utils/Debug.hpp>

/**/
#include <drivers/ethernet/dwmac.hpp>

namespace Init {
void init() {
    bool BSP = CPU::id() == Traits<Machine>::BSP;
    if (BSP) {
        Console::init();
        TraceIn();
        Memory::init();
        Timer::init();
        // Task::init();
        Thread::init();
        Application::init();
        TraceOut();
        Ethernet::init();
    }

    CPU::barrier();

    if (CPU::id() < Traits<Machine>::CPUS) {
        Thread::run();
    }
    for (;;)
        ;
}
} // namespace Init

extern "C" __attribute__((naked, used, noinline, section(".init"))) void _init() {
    CPU::setup();
    CPU::init();
    BSS::init();
    // MMU::init();
    ////    RV64::csrs<RV64::Supervisor::IE>(RV64::Supervisor::TI);
    ////    Console::print(CPU::id());
    Init::init();
}
