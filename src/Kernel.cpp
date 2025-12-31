#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

class Init {
  public:
    static void init() {
        bool BSP = Machine::CPU::id() == Traits<CPUS>::BSP;
        if (BSP) {
            Console::init();
            TraceIn();
            Memory::init();
            Timer::init();
            Application::init();
            Thread::init();
            TraceOut();
        }

        Machine::CPU::barrier();
        Thread::run();
        Machine::CPU::halt();
    }
};

extern "C" __attribute__((naked, used, noinline, section(".init"))) void _init() {
    Machine::CPU::probe();
    Machine::CPU::jmode();
    Machine::CPU::init();
    Init::init();
}
