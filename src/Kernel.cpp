#include <Application.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <machine/Machine.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

class System {
  public:
    static void init() {
        bool BSP = CPU::id() == Traits<CPUS>::BSP;
        if (BSP) TraceIn();

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

class Hypervisor {
    static_assert(Traits<CPUS>::ACTIVE == 1);

  public:
    static void init() {
        TraceIn();

        CPU::barrier();

        Machine::init();

        reinterpret_cast<void (*)(unsigned int, uintptr_t)>(Traits<MemoryMap>::ApplicationAddr)(0, 0x82200000ULL);

        CPU::halt();
    }
};

extern "C" __attribute__((naked, used, noinline, section(".init"))) void _init() {
    CPU::init();
    if constexpr (!Traits<System>::Hypervisor)
        System::init();
    else
        Hypervisor::init();
}
