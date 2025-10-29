#include <IO/Debug.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

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
            booting = false;
        }
        while (booting);
        if (Machine::CPU::core() < Traits::Machine::CPUS) {
            Machine::MMU::init();
        }
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

extern "C" void *memset(void *s, int c, unsigned long n) {
    unsigned char *p = static_cast<unsigned char *>(s);
    while (n-- > 0) {
        *p++ = static_cast<unsigned char>(c);
    }
    return s;
}
