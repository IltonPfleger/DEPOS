#include <architecture/Timer.hpp>
#include <machine/Machine.hpp>

#include <kernel/ApplicationHandler.hpp>
#include <kernel/Thread.hpp>
#include <kernel/memory/BSS.hpp>
#include <kernel/memory/Memory.hpp>

using namespace DEPOS;

extern "C" void init() {
    if (CPU::id() == Traits<CPU>::BSP) TraceIn();

    CPU::barrier();

    if constexpr (Traits<Timer>::Enable) Timer::init();

    if (CPU::id() == Traits<CPU>::BSP) {
        Memory::init();
        Thread::init();
        ApplicationHandler::init();
    }

    CPU::barrier();

    Thread::run();
}

extern "C" __attribute__((optimize("O0"), naked, used, section(".init"))) void _init() {
    CPU::init();
    Machine::init();
    init();
}
