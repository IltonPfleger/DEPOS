#include <Thread.hpp>
#include <Timer.hpp>
#include <application/Application.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

class System {
  public:
    __attribute__((noinline)) static void init() {
        volatile bool BSP = CPU::id() == Traits<CPUS>::BSP;
        if (BSP) TraceIn();

        Trace(CPU::id());

        CPU::barrier();

        Machine::init();

        if (BSP) {
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
    System::init();
}
