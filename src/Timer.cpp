#include <Thread.hpp>
#include <Timer.hpp>
#include <Traits.hpp>

void Timer::init() {
    if constexpr (Traits<Scheduler<Thread>>::Criterion::Preemptive) {
        s_scheduler.duration = Traits<Timer>::Frequency / Traits<Scheduler<Thread>>::Frequency;
        for (auto &e : s_scheduler.current)
            e = s_scheduler.duration;
        // s_scheduler.current[Machine::CPU::core()] = s_scheduler.duration;
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
        if (--s_scheduler.current[core] == 0) {
            s_scheduler.current[core] = s_scheduler.duration;
            Thread::reschedule();
        }
    }
}
