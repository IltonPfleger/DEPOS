#include <Application.hpp>
#include <IO/Debug.hpp>
#include <Machine.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <memory/Memory.hpp>

static volatile bool booting  = true;
static volatile bool starting = true;

namespace Init {
    void init() {
        bool BSP = Machine::CPU::core() == Traits::Machine::BSP;
        if (BSP) {
            Console::init();
            TraceIn();
            Memory::init();
            Task::init();
            Application::init();
            booting = false;
        }
        while (booting);
        if (BSP) {
            Thread::init();
            TraceOut();
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

// extern "C" __attribute__((naked, section(".boot"))) void kboot() {
extern "C" __attribute__((naked, used, noinline, section(".init"))) void _init() {
    Machine::CPU::setup();
    Machine::CPU::init();
    Init::init();
}
