#include <Alarm.hpp>

void Alarm::delay(unsigned int seconds) {
    unsigned long ticks = seconds * Traits<Alarm>::Frequency;

    Delay d{Thread::Queue{}, ticks, nullptr};

    s_spin.acquire();
    if (!s_delays || ticks < s_delays->ticks) {
        if (s_delays) s_delays->ticks -= ticks;

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

        if (current->next) current->next->ticks -= remaining;

        d.next = current->next;
        current->next = &d;
    }

    Thread::sleep(d.queue, s_spin);
}

void Alarm::handler() {
    s_spin.acquire();
    if (s_delays && --s_delays->ticks <= 0) {
        Thread::wakeup(s_delays->queue);
        s_delays = s_delays->next;
    }
    s_spin.release();
}
