#pragma once
// #include <Alarm.hpp>
#include <Traits.hpp>
// #include <Types.hpp>

class Timer {
    struct Channel {
        unsigned long _default;
        unsigned long _current[Traits<Machine>::CPUS];
    };

    static inline Channel _scheduler;

  public:
    static void handler(unsigned long);
    static void init() {
        if constexpr (Traits<Scheduler<Thread>>::Preemptive) {
            _scheduler._default =
                Traits<Timer>::Frequency / Traits<Scheduler<Thread>>::Frequency;
            for (auto &e : _scheduler._current)
                e = _scheduler._default;
            //_scheduler._current[Machine::CPU::core()] = _scheduler._default;
        }

        // if constexpr (Traits<Alarm>::Enable) {
        //     _channels[ALARM]._default              = Traits<Timer>::Frequency
        //     / Traits<Alarm>::Frequency;
        //     _channels[ALARM]._current[CPU::core()] =
        //     _channels[ALARM]._default;
        // }
    }
};
