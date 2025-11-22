#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

namespace Init {
void init() {
    bool BSP = CPU::id() == Traits<Machine>::BSP;
    if (BSP) {
        Console::init();
        TraceIn();
        Memory::init();
        Timer::init();
        // Task::init();
        // Application::init();
        Thread::init();
        TraceOut();
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
    Init::init();
}
