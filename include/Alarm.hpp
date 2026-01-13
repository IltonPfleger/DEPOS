#pragma once

#include <Thread.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>

class Alarm {
    struct Delay {
        Thread::Queue queue;
        unsigned long ticks;
        Delay *next;
    };

  public:
    static void delay(unsigned int seconds) {
        unsigned long ticks = seconds * Traits<Alarm>::Frequency;

        Delay d{Thread::Queue{}, ticks, nullptr};

        s_lock.lock();
        if (!s_delays || ticks < s_delays->ticks) {
            if (s_delays)
                s_delays->ticks -= ticks;

            d.next = s_delays;
            s_delays = &d;
        } else {
            Delay *current = s_delays;
            unsigned long remaining = ticks;

            while (current->next && remaining >= current->next->ticks) {
                remaining -= current->next->ticks;
                current = current->next;
            }

            d.ticks = remaining;

            if (current->next)
                current->next->ticks -= remaining;

            d.next = current->next;
            current->next = &d;
        }

        Thread::sleep(d.queue, s_lock);
    }

    static void handler() {
        s_lock.lock();
        if (s_delays && --s_delays->ticks <= 0) {
            Thread::wakeup(s_delays->queue);
            s_delays = s_delays->next;
        }
        s_lock.unlock();
    }

  private:
    static inline Delay *s_delays = nullptr;
    static inline Spin s_lock;
};
