#include <Alarm.hpp>
#include <Thread.hpp>
#include <Timer.hpp>
#include <Traits.hpp>
#include <machine/Traits.hpp>

void Timer::init() {
    if constexpr (Traits<Alarm>::Enable) {
        s_alarm.duration = Traits<Timer>::Frequency / Traits<Alarm>::Frequency;
        s_alarm.current[Traits<CPUS>::BSP] = s_alarm.duration;
    }

    if constexpr (Traits<Scheduler<Thread>>::Preemptive) {
        s_scheduler.duration = Traits<Timer>::Frequency / Traits<Scheduler<Thread>>::Frequency;
        for (auto &e : s_scheduler.current)
            e = s_scheduler.duration;
    }
}

void Timer::handler(unsigned long core) {
    if constexpr (Traits<Alarm>::Enable) {
        auto &counter = s_alarm;
        if (core == Traits<CPUS>::BSP && --counter.current[core] == 0) {
            counter.current[core] = counter.duration;
            Alarm::handler();
        }
    }

    if constexpr (Traits<Scheduler<Thread>>::Preemptive) {
        auto &counter = s_scheduler;
        if (--counter.current[core] == 0) {
            counter.current[core] = counter.duration;
            Thread::reschedule();
        }
    }
}
