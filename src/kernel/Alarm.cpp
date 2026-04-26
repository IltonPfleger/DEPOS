#include <architecture/Timer.hpp>
#include <kernel/Alarm.hpp>

namespace DEPOS {

bool Alarm::elapsed(Microsecond us) { return static_cast<intmax_t>(Timer::now() - us) >= 0; }

void Alarm::at(Microsecond us) {
    int core = CPU::id();
    Thread::Queue queue;
    Spin spin;

    Link link(&queue, us);
    s_delays[core].insert(&link);
    Thread::sleep(&queue, &spin);
}

void Alarm::udelay(Microsecond us) { Alarm::at(Timer::now() + us); }

void Alarm::onTick() {
    int core   = CPU::id();
    List &list = s_delays[core];

    Link *head = list.head();
    while (head && elapsed(head->criterion())) {
        Thread::wakeup(head->value());
        list.remove();
        head = list.head();
    }
}

} // namespace DEPOS
