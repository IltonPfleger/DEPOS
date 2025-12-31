#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

class Init {
  public:
    static void init() {
        bool BSP = CPU::id() == Traits<Machine>::BSP;
        if (BSP) {
            Console::init();
            TraceIn();
            Memory::init();
            Timer::init();
            Application::init();
            Thread::init();
            TraceOut();
        }

        CPU::barrier();

        if (CPU::id() < Traits<Machine>::CPUS) {
            Thread::run();
        }

        CPU::halt();
    }
};

extern "C" __attribute__((naked, used, noinline, section(".init"))) void _init() {
    CPU::probe();
    CPU::jmode();
    CPU::init();
    Init::init();
}
