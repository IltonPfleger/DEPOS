#include <architecture/Timer.hpp>
#include <machine/Machine.hpp>

#include <Application.hpp>
#include <Thread.hpp>
#include <memory/Memory.hpp>

using namespace DEPOS;

extern "C" void init() {
    if (CPU::id() == Traits<CPU>::BSP) {
        Console::println(' ');
        TraceIn();
    }

    CPU::barrier();

    if constexpr (Traits<Timer>::Enable) Timer::init();

    if (CPU::id() == Traits<CPU>::BSP) {
        Memory::init();
        Thread::init();
        Application::init();
    }

    CPU::barrier();
    Thread::run();
}

extern "C" __attribute__((optimize("O0"), naked, used, section(".init"))) void _init() {
    CPU::init();
    Machine::init();
    init();
}
