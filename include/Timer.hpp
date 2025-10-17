#pragma once
#include <Alarm.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Traits.hpp>

namespace Timer {
    struct Channel {
        Tick _default;
        Tick _current[Traits::Machine::CPUS];
    };

    static inline Channel _scheduler;

    void init() {
        if constexpr (Traits::Scheduler<Thread>::Criterion::Timed) {
            _scheduler._default                       = Traits::Timer::Frequency / Traits::Scheduler<Thread>::Frequency;
            _scheduler._current[Machine::CPU::core()] = _scheduler._default;
        }

        // if constexpr (Traits::Alarm::Enable) {
        //     _channels[ALARM]._default              = Traits::Timer::Frequency / Traits::Alarm::Frequency;
        //     _channels[ALARM]._current[CPU::core()] = _channels[ALARM]._default;
        // }

        // CPU::Interrupt::Timer::enable();
    }

    void handler(unsigned int core) {
        // auto core = Machine::CPU::core();
        //  reset(core);
        //     if constexpr (Traits::Alarm::Enable) {
        //         if (--_channels[ALARM]._current[CPU::core()] == 0) {
        //             _channels[ALARM]._current[CPU::core()] = _channels[ALARM]._default;
        //             Alarm::handler();
        //         }
        //     }

        if constexpr (Traits::Scheduler<Thread>::Criterion::Timed) {
            if (--_scheduler._current[core] == 0) {
                _scheduler._current[core] = _scheduler._default;
                Thread::reschedule();
            }
        }
    }
}
