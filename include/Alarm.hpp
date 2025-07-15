#pragma once
#include <IO/Logger.hpp>
#include <Semaphore.hpp>
#include <Traits.hpp>

template <typename T>
concept AlarmEnable = Traits<T>::Enable;

template <typename T>
concept AlarmMHz = (Traits<T>::Frequency >= Traits<Timer>::MHz);

struct Alarm {
    struct Delay {
        RawSemaphore semaphore;
        unsigned long long clock;
        Delay *next;
    };

    static inline Delay *delays = nullptr;

    template <AlarmEnable T = Alarm, AlarmMHz U = T>
    static void usleep(auto useconds) {
        auto duration = useconds * Machine::CLINT::CLOCK / Traits<Timer>::MHz;
        Delay entry{RawSemaphore(0), Machine::CLINT::MTIME + duration, nullptr};

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
        entry.semaphore.p();
    }

    static void handler() {
        if (delays && Machine::CLINT::MTIME >= delays->clock) {
            delays->semaphore.v();
            delays = delays->next;
        }
    }
};
