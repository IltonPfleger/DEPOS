#pragma once
#include <IO/Logger.hpp>
#include <Semaphore.hpp>
#include <Traits.hpp>

struct Alarm {
    struct Delay {
        RawSemaphore semaphore;
        unsigned long long clock;
        Delay *next;
    };

    static inline Delay *delays = nullptr;

    template <typename T = void>
        requires(Traits<Alarm>::Enable && Traits<Alarm>::Frequency >= 1'000'000)
    static void usleep(auto useconds) {
        auto duration = useconds * Machine::CLINT::CLOCK / 1'000'000;
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

    // template <typename T = void>
    //     requires(Traits<Alarm>::Enable)
    // static void delay(unsigned long seconds) {
    //     CPU::Interrupt::disable();
    //     unsigned long ticks = seconds * Traits<Alarm>::Frequency;
    //     Delay entry{RawSemaphore(0), ticks, nullptr};

    //    if (!delays || entry.clock < delays->clock) {
    //        if (delays) delays->clock -= entry.clock;
    //        entry.next = delays;
    //        delays     = &entry;
    //    } else {
    //        unsigned long sum = delays->clock;
    //        Delay *current    = delays;

    //        while (current->next && sum + current->next->clock < entry.clock) {
    //            current = current->next;
    //            sum += current->clock;
    //        }

    //        entry.clock -= sum;

    //        if (current->next) current->next->clock -= entry.clock;

    //        entry.next    = current->next;
    //        current->next = &entry;
    //    }
    //    entry.semaphore.p();
    //    delays = delays->next;
    //}

    static void handler() {
        if (delays && Machine::CLINT::MTIME >= delays->clock) {
            delays->semaphore.v();
            delays = delays->next;
        }
    }
};
