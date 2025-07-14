#pragma once
#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Meta.hpp>
#include <Semaphore.hpp>
#include <Traits.hpp>

struct Alarm {
    struct Delay {
        RawSemaphore semaphore;
        unsigned long long value;
        Delay *next;
    };

    static inline Delay *delays = nullptr;

    template <typename T = void>
        requires(Traits<Alarm>::Enable && Traits<Alarm>::Frequency >= Traits<Timer>::MHz)
    static void usleep(unsigned long long useconds) {
        auto ticks = useconds;  // * (Traits<Alarm>::Frequency / Traits<Timer>::MHz);
        // Logger::println("%u\n", ticks);
        Delay entry{RawSemaphore(0), ticks, nullptr};

        //CPU::Interrupt::disable();
        // if (!delays || entry.value < delays->value) {
        //     if (delays) delays->value -= entry.value;
        //     entry.next = delays;
        //     delays     = &entry;
        // } else {
        //     auto sum       = delays->value;
        //     Delay *current = delays;

        //    while (current->next && sum + current->next->value < entry.value) {
        //        current = current->next;
        //        sum += current->value;
        //    }

        //    entry.value -= sum;

        //    if (current->next) current->next->value -= entry.value;

        //    entry.next    = current->next;
        //    current->next = &entry;
        //}
        delays = &entry;
        entry.semaphore.p();
    }

    static void handler() {
        if (delays && --delays->value == 0) {
            delays->semaphore.v();
            delays = delays->next;
        }
    }
};
