#include <Thread.hpp>

#include <abstractions/Timer.hpp>

#include <application/Application.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>
#include <utils/Dispatcher.hpp>

extern "C" __attribute__((optimize("O0"))) void init() {
    if (CPU::id() == Traits<CPU>::BSP) TraceIn();

    CPU::barrier();

    Machine::init();

    CPU::barrier();

    if constexpr (Traits<Timer>::Enable) Timer::init();

    if (CPU::id() == Traits<CPU>::BSP) {
        Memory::init();
        Application::init();
        Thread::init();
    }

    CPU::barrier();
    if (CPU::id() == Traits<CPU>::BSP) TraceOut("Booting...");
    Thread::run();
}

extern "C" __attribute__((naked, used, section(".init"))) void _init() {
    CPU::init();
    init();
}
