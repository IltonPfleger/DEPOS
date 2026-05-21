#include <Application.hpp>
#include <Thread.hpp>
#include <architecture/Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <monitor/Monitor.hpp>

using namespace DEPOS;

extern "C" void init() {
    if (CPU::id() == Traits<CPU>::BSP) {
        Console::println(' ');
        TraceIn();
        Memory::init();
        Thread::init();
        Application::init();
    }

    if constexpr (Traits<Timer>::Enable) Timer::init();

    CPU::barrier();

    Thread::run();
}

extern "C" __attribute__((optimize("O0"), naked, used, section(".init"))) void _init() {
    CPU::init();
    Machine::init();
    init();
}
