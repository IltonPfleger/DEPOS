#pragma once

#include <Thread.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>

class Alarm {
    struct Delay {
        Thread::Queue queue;
        unsigned long end;
        Delay *next;
    };

  public:
    static void delay(unsigned int seconds) {
        unsigned long value = seconds * Traits<Alarm>::Frequency;
        unsigned long now = Machine::Timer::now();
        unsigned long end = now + value;

        Delay delay{Thread::Queue{}, end, nullptr};

        s_lock.lock();
        if (!s_delays || delay.end < s_delays->end) {
            delay.next = s_delays;
            s_delays = &delay;
        } else {
            Delay *current = s_delays;

            while (current->next && current->next->end <= delay.end) {
                current = current->next;
            }
            delay.next = current->next;
            current->next = &delay;
        }

        Thread::sleep(delay.queue, s_lock);
    }

    static void handler() {
        if (s_delays && Machine::Timer::now() >= s_delays->end) {
            Thread::wakeup(s_delays->queue);
            s_delays = s_delays->next;
        }
    }

  private:
    static inline Delay *s_delays = nullptr;
    static inline Spin s_lock;
};
