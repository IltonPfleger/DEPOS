#include <Thread.hpp>
#include <Timer.hpp>
#include <application/Application.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

extern "C" __attribute__((optimize("O0"))) void init() {
    if (CPU::id() == Traits<CPUS>::BSP) TraceIn();

    CPU::barrier();

    Machine::init();

    CPU::barrier();

    if (CPU::id() == Traits<CPUS>::BSP) {
        Memory::init();
        Timer::init();
        Application::init();
        Thread::init();
        TraceOut();
    }

    CPU::barrier();
    Thread::run();
}

extern "C" __attribute__((naked, used, section(".init"))) void _init() {
    CPU::init();
    init();
}
