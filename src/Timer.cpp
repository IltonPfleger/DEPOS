#include <Thread.hpp>
#include <Timer.hpp>
#include <Traits.hpp>

void Timer::init() {
    if constexpr (Traits<Scheduler<Thread>>::Preemptive) {
        _scheduler._default =
            Traits<Timer>::Frequency / Traits<Scheduler<Thread>>::Frequency;
        for (auto &e : _scheduler._current)
            e = _scheduler._default;
        //_scheduler._current[Machine::CPU::core()] = _scheduler._default;
    }
}

void Timer::handler(unsigned long core) {
    // auto core = Machine::CPU::core();
    //  reset(core);
    //     if constexpr (Traits<Alarm>::Enable) {
    //         if (--_channels[ALARM]._current[CPU::core()] == 0) {
    //             _channels[ALARM]._current[CPU::core()] =
    //             _channels[ALARM]._default; Alarm::handler();
    //         }
    //     }

    if constexpr (Traits<Scheduler<Thread>>::Preemptive) {
        if (--_scheduler._current[core] == 0) {
            _scheduler._current[core] = _scheduler._default;
            Thread::reschedule();
        }
    }
}
