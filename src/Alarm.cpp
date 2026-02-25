#include <Alarm.hpp>

void Alarm::udelay(unsigned int microseconds) {
    unsigned long ticks = (microseconds * Traits<Alarm>::Frequency) / 1'000'000;
    if (ticks == 0) return;

    Delay d{Thread::Queue{}, ticks, nullptr};

    bool enabled = CPU::Interruptions::disable();
    s_spin.acquire();

    if (!s_delays || d.ticks < s_delays->ticks) {
        if (s_delays) s_delays->ticks -= d.ticks;
        d.next = s_delays;
        s_delays = &d;
    } else {
        Delay *prev = s_delays;
        d.ticks -= prev->ticks;

        while (prev->next && d.ticks >= prev->next->ticks) {
            d.ticks -= prev->next->ticks;
            prev = prev->next;
        }

        d.next = prev->next;
        if (d.next) d.next->ticks -= d.ticks;
        prev->next = &d;
    }

    Thread::sleep(&d.queue, &s_spin);
    if (enabled) CPU::Interruptions::enable();
}

void Alarm::handler() {
    s_spin.acquire();

    if (s_delays) {
        if (s_delays->ticks > 0) {
            s_delays->ticks--;
        }

        while (s_delays && s_delays->ticks <= 0) {
            Delay *expired = s_delays;
            s_delays = s_delays->next;
            Thread::wakeup(&expired->queue);
        }
    }

    s_spin.release();
}
