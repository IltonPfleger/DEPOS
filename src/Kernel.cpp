#include <Thread.hpp>
#include <Timer.hpp>
#include <application/Application.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

class System {
  public:
    static void init() {
        if (CPU::id() == Traits<CPUS>::BSP) TraceIn();

        CPU::barrier();

        Machine::init();

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
};

#pragma GCC optimize("O0")
extern "C" __attribute__((naked, used, section(".init"))) void _init() {
    CPU::init();
    System::init();
}
