#include <IO/Debug.hpp>
#include <Language.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

static volatile bool boot = true;

namespace Kernel {
    void init() {
        if (Machine::CPU::core() == Traits::Machine::BSP) {
            Logger::init();
            Logger::println("\n[Kernel]\n");
            Memory::init();
            Thread::init();
            Logger::println("Done!\n");
            boot = false;
        }
        while (boot);
        if (Machine::CPU::core() < Traits::Machine::CPUS) {
            Timer::init();
            Thread::run();
        }
        for (;;);
    }
}

__attribute__((naked, section(".boot"))) void kboot() { Machine::CPU::init(); }
