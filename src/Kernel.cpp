#include <IO/Debug.hpp>
#include <Machine.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <memory/Memory.hpp>

static volatile bool booting  = true;
static volatile bool starting = true;

namespace Kernel {
    void init() {
        bool BSP = Machine::CPU::core() == Traits::Machine::BSP;
        if (BSP) {
            Console::init();
            TRACE("\n");
            TRACE(__PRETTY_FUNCTION__, "{");
            TRACE("\n");
            Memory::init();
            // Task::init();
            booting = false;
        }
        while (booting);
        if (BSP) {
            Thread::init();
            TRACE("}\n");
            starting = false;
        }
        while (starting);
        if (Machine::CPU::core() < Traits::Machine::CPUS) {
            Timer::init();
            Thread::run();
        }
        for (;;);
    }
}

extern "C" __attribute__((naked, section(".boot"))) void kboot() {
    Machine::CPU::setup();
    Machine::CPU::init();
}
