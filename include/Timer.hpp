#pragma once
#include <Alarm.hpp>
#include <CPU.hpp>
#include <Machine.hpp>
#include <Scheduler/Scheduler.hpp>
#include <Thread.hpp>
#include <Traits.hpp>

volatile uintptr_t &MTIME    = *reinterpret_cast<volatile uintptr_t *>(Machine::CLINT::ADDR + 0xBFF8);
volatile uintptr_t &MTIMECMP = *reinterpret_cast<volatile uintptr_t *>(Machine::CLINT::ADDR + 0x4000);

struct Timer {
    struct Channel {
        typedef void (*Handler)(void);
        enum { SCHEDULER, ALARM };
        uintptr_t initial;
        uintptr_t current;
        void (*handler)();
    };

    static inline struct Channel CHANNELS[2];

    static void reset() {
        uintptr_t now = MTIME;
        MTIMECMP      = now + (Machine::CLINT::CLOCK / Traits<Timer>::Frequency);
    }

    static void init() {
        if constexpr (Traits<Scheduler<Thread>>::Criterion::Timed) {
            CHANNELS[Channel::SCHEDULER].handler = Thread::reschedule;
            unsigned long initial                = Traits<Timer>::Frequency / Traits<Scheduler<Thread>>::Frequency;
            CHANNELS[Channel::SCHEDULER].initial = initial;
            CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
        }

        if constexpr (Traits<Alarm>::Enable) {
            CHANNELS[Channel::ALARM].handler = Alarm::handler;
            CHANNELS[Channel::ALARM].initial = Traits<Timer>::Frequency / Traits<Alarm>::Frequency;
            CHANNELS[Channel::ALARM].current = CHANNELS[Channel::ALARM].initial;
        }

        if constexpr (Traits<Timer>::Enable) {
            reset();
            CPU::Interrupt::Timer::enable();
        }
    }

    static void handler() {
        reset();
        if constexpr (Traits<Scheduler<Thread>>::Criterion::Timed) {
            if (--(CHANNELS[Channel::SCHEDULER].current) == 0) {
                CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
                CHANNELS[Channel::SCHEDULER].handler();
            }
        }

        if constexpr (Traits<Alarm>::Enable) {
            if (--(CHANNELS[Channel::ALARM].current) == 0) {
                CHANNELS[Channel::ALARM].current = CHANNELS[Channel::ALARM].initial;
                CHANNELS[Channel::ALARM].handler();
            }
        }
        reset();
    }
};
