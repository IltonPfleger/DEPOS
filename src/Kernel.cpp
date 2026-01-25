#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

class Init {
  public:
    static void init() {
        bool BSP = CPU::id() == Traits<CPUS>::BSP;
        if (BSP) TraceIn();

        CPU::barrier();

        Machine::init();

        if (BSP) {
            TraceIn();
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

extern "C" __attribute__((naked, used, noinline, section(".init"))) void _init() {
    CPU::init();
    Init::init();
}
