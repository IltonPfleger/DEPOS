#include <Alarm.hpp>
#include <architecture/Timer.hpp>

namespace DEPOS {

bool Alarm::elapsed(Microsecond us) { return static_cast<intmax_t>(Timer::now() - us) >= 0; }

void Alarm::at(Microsecond us) {
    Thread::Queue queue;
    Link link(&queue, us);
    s_spin.acquire();
    s_delays.insert(&link);
    Thread::sleep(&queue, &s_spin);
}

void Alarm::udelay(Microsecond us) { Alarm::at(Timer::now() + us); }

void Alarm::handler() {
    s_spin.acquire();

    Link *head = s_delays.head();
    while (head && elapsed(head->criterion())) {
        Thread::wakeup(head->value());
        s_delays.remove();
        head = s_delays.head();
    }

    s_spin.release();
}

} // namespace DEPOS
