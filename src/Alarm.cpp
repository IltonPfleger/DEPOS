#include <Alarm.hpp>
#include <architecture/Timer.hpp>

namespace DEPOS {

bool Alarm::elapsed(Microsecond us) { return static_cast<intmax_t>(Timer::us() - us) >= 0; }

void Alarm::at(Microsecond us) {
    int core = CPU::id();
    Link link(Thread::Queue{}, us);

    bool enabled = CPU::Interrupt::disable();
    delays_[core].insert(&link);
    if (enabled) CPU::Interrupt::enable();

    Spin stub;
    Thread::sleep(&link.value(), &stub);
}

void Alarm::udelay(Microsecond us) { Alarm::at(Timer::us() + us); }

void Alarm::onTick() {
    int core   = CPU::id();
    List &list = delays_[core];

    Link *head = list.head();
    while (head && elapsed(head->criterion())) {
        Thread::wakeup(&head->value());
        list.remove();
        head = list.head();
    }
}

} // namespace DEPOS
