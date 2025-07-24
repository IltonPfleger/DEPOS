#pragma once
#include <Thread.hpp>
#include <Traits.hpp>

template <typename T>
concept AlarmEnable = Traits::Alarm::Enable;

template <typename T>
concept AlarmMHz = (Traits::Alarm::Frequency >= 1'000'000);

struct Alarm {
    struct Delay {
        Thread::Queue waiting;
        unsigned long long clock;
        Delay *next;
    };

    static inline Delay *delays = nullptr;
    static inline Spin lock;

    static uintptr_t utime() { return (*Machine::CLINT::MTIME * 1'000'000) / Machine::CLINT::CLOCK; }

    template <AlarmEnable T = Alarm, AlarmMHz U = T>
    static void usleep(auto useconds) {
        auto duration = useconds * Machine::CLINT::CLOCK / 1'000'000;
        Delay entry{Thread::Queue{}, *Machine::CLINT::MTIME + duration, nullptr};

        CPU::Interrupt::disable();
        lock.lock();
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
        lock.unlock();
        CPU::Interrupt::enable();
        Thread::sleep(&entry.waiting);
    }

    static void handler() {
        lock.lock();
        if (delays && *Machine::CLINT::MTIME >= delays->clock) {
            Thread::wakeup(&delays->waiting);
            delays = delays->next;
        }
        lock.unlock();
    }
};
