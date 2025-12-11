#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/BSS.hpp>
#include <utils/Debug.hpp>

class Init {
  public:
    static void init() {
        if (CPU::id() == Traits<Machine>::BSP) {
            Console::init();
            TraceIn();
            Memory::init();
            Timer::init();
            Thread::init();
            Application::init();
            TraceOut();
        }

        CPU::barrier();

        if (CPU::id() < Traits<Machine>::CPUS) {
            Thread::run();
        }
        CPU::idle();
    }
};

extern "C" __attribute__((naked, used, aligned(4), section(".init"))) void _init() {
    CPU::setup();
    CPU::init();
    MMU::init();
    BSS::init();
    Init::init();
}
