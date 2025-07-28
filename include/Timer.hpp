#pragma once
#include <Alarm.hpp>
#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Traits.hpp>

template <bool enabled = Traits::Timer::Enable>
class _Timer {
    static void handler() {}

   public:
    static void init() {}
};

template <>
class _Timer<true> {
    enum { SCHEDULER, ALARM, COUNT };
    struct Channel {
        Tick _initial;
        Tick _current[Machine::CPUS];
    };

    static inline Channel _channels[COUNT];

    static void reset() {
        *reinterpret_cast<volatile uintmax_t *>(Machine::CLINT::MTIMECMP + (CPU::core() * 8)) =
            *Machine::CLINT::MTIME + (Machine::CLINT::CLOCK / Traits::Timer::Frequency);
    }

   public:
    static void init() {
        if constexpr (Traits::Scheduler<Thread>::Criterion::Timed) {
            _channels[SCHEDULER]._initial = Traits::Timer::Frequency / Traits::Scheduler<Thread>::Frequency;
            _channels[SCHEDULER]._current[CPU::core()] = _channels[SCHEDULER]._initial;
        }

        if constexpr (Traits::Alarm::Enable) {
            _channels[ALARM]._initial              = Traits::Timer::Frequency / Traits::Alarm::Frequency;
            _channels[ALARM]._current[CPU::core()] = _channels[ALARM]._initial;
        }

        CPU::Interrupt::Timer::enable();
    }

    static void handler() {
        reset();
        if constexpr (Traits::Alarm::Enable) {
            if (--_channels[ALARM]._current[CPU::core()] == 0) {
                _channels[ALARM]._current[CPU::core()] = _channels[ALARM]._initial;
                Alarm::handler();
            }
        }

        if constexpr (Traits::Scheduler<Thread>::Criterion::Timed) {
            if (--_channels[SCHEDULER]._current[CPU::core()] == 0) {
                _channels[SCHEDULER]._current[CPU::core()] = _channels[SCHEDULER]._initial;
                Thread::reschedule();
            }
        }
    }
};

using Timer = _Timer<>;
