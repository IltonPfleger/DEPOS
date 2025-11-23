#include <Thread.hpp>
#include <Timer.hpp>
#include <Traits.hpp>

void Timer::init() {
    if constexpr (Traits<Scheduler<Thread>>::Criterion::Preemptive) {
        scheduler_s.duration = Traits<Timer>::Frequency / Traits<Scheduler<Thread>>::Frequency;
        for (auto &e : scheduler_s.current)
            e = scheduler_s.duration;
        // scheduler_s.current[Machine::CPU::core()] = scheduler_s.duration;
    }
}

void Timer::handler(unsigned long core) {
    // auto core = Machine::CPU::core();
    //  reset(core);
    //     if constexpr (Traits<Alarm>::Enable) {
    //         if (--_channels[ALARM].current[CPU::core()] == 0) {
    //             _channels[ALARM].current[CPU::core()] =
    //             _channels[ALARM].duration; Alarm::handler();
    //         }
    //     }

    if constexpr (Traits<Scheduler<Thread>>::Criterion::Preemptive) {
        if (--scheduler_s.current[core] == 0) {
            scheduler_s.current[core] = scheduler_s.duration;
            Thread::reschedule();
        }
    }
}
