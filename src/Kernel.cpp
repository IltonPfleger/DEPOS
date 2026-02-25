#include <Thread.hpp>
#include <application/Application.hpp>
#include <architecture/Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>
#include <utils/Debug.hpp>
#include <utils/Dispatcher.hpp>

using namespace DEPOS;

extern "C" void init() {
    if (CPU::id() == Traits<CPU>::BSP) {
        Console::init();
        TraceIn();
    }

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
    Thread::run();
}

extern "C" __attribute__((optimize("O0"), naked, used, section(".init"))) void _init() {
    CPU::init();
    BSS::init();
    Console::cout << "OI";
    init();
}
