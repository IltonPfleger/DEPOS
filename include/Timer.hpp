#pragma once
#include <Alarm.hpp>
#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Traits.hpp>

namespace Timer {
    struct Channel {
        Tick _initial;
        Tick _current[Machine::CPUS];
    };

    static inline Channel _scheduler;

    static void reset() {
        static constexpr uintmax_t ticks = Machine::CLINT::CLOCK / Traits::Timer::Frequency;
        *reinterpret_cast<volatile uintmax_t *>(Machine::CLINT::MTIMECMP + (CPU::core() * 8)) =
            *Machine::CLINT::MTIME + ticks;
    }

    void init() {
        if constexpr (Traits::Scheduler<Thread>::Criterion::Timed) {
            _scheduler._initial              = Traits::Timer::Frequency / Traits::Scheduler<Thread>::Frequency;
            _scheduler._current[CPU::core()] = _scheduler._initial;
        }

        // if constexpr (Traits::Alarm::Enable) {
        //     _channels[ALARM]._initial              = Traits::Timer::Frequency / Traits::Alarm::Frequency;
        //     _channels[ALARM]._current[CPU::core()] = _channels[ALARM]._initial;
        // }

        CPU::Interrupt::Timer::enable();
    }

    void handler() {
        reset();
        //  if constexpr (Traits::Alarm::Enable) {
        //      if (--_channels[ALARM]._current[CPU::core()] == 0) {
        //          _channels[ALARM]._current[CPU::core()] = _channels[ALARM]._initial;
        //          Alarm::handler();
        //      }
        //  }

        if constexpr (Traits::Scheduler<Thread>::Criterion::Timed) {
            if (--_scheduler._current[CPU::core()] == 0) {
                _scheduler._current[CPU::core()] = _scheduler._initial;
                Thread::reschedule();
            }
        }
    }
}
