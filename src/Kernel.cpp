#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

static volatile bool booting = true;
static volatile bool starting = true;

namespace Init {
void init() {
    bool BSP = CPU::id() == Traits<Machine>::BSP;
    if (BSP) {
        Console::init();
        TraceIn();
        Memory::init();
        Timer::init();
        // Machine::MMU::KernelPageTable::init();
        //  Task::init();
        Application::init();
        booting = false;
    }
    while (booting)
        ;
    if (BSP) {
        Thread::init();
        TraceOut();
        starting = false;
    }
    while (starting)
        ;
    if (CPU::id() < Traits<Machine>::CPUS) {
        Thread::run();
    }
    for (;;)
        ;
}
} // namespace Init

extern "C" __attribute__((naked, used, noinline, section(".init"))) void
_init() {
    CPU::setup();
    CPU::init();
    Init::init();
}
