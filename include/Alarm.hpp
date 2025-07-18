#pragma once
#include <Thread.hpp>
#include <Traits.hpp>

template <typename T>
concept AlarmEnable = Traits<T>::Enable;

template <typename T>
concept AlarmMHz = (Traits<T>::Frequency >= Traits<Timer>::MHz);

struct Alarm {
    struct Delay {
        Thread::Queue waiting;
        unsigned long long clock;
        Delay *next;
    };

    static inline Delay *delays = nullptr;

    static uintptr_t utime() { return (Machine::CLINT::MTIME * 1'000'000) / Machine::CLINT::CLOCK; }

    template <AlarmEnable T = Alarm, AlarmMHz U = T>
    static void usleep(auto useconds) {
        auto duration = useconds * Machine::CLINT::CLOCK / Traits<Timer>::MHz;
        Delay entry{Thread::Queue{}, Machine::CLINT::MTIME + duration, nullptr};

        CPU::Interrupt::disable();
        if (!delays || entry.clock < delays->clock) {
            entry.next = delays;
            delays     = &entry;
        } else {
            Delay *current = delays;

            while (current->next && current->next->clock <= entry.clock) {
                current = current->next;
            }
            entry.next    = current->next;
            current->next = &entry;
        }
        Thread::sleep(&entry.waiting);
    }

    static void handler() {
        if (delays && Machine::CLINT::MTIME >= delays->clock) {
            Thread::wakeup(&delays->waiting);
            delays = delays->next;
        }
    }
};
