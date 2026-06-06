#include <Alarm.hpp>
#include <architecture/Timer.hpp>
#include <utility/Console.hpp>

namespace DEPOS {

bool Alarm::elapsed(Microsecond us) { return static_cast<intmax_t>(Timer::us() - us) >= 0; }

void Alarm::at(Microsecond us) {
    Link link(Thread::Queue{}, us);

    bool enabled = CPU::Interrupt::disable();

    int core = CPU::id();
    delays_[core].insert(&link);

    Thread::sleep(&link.value(), nullptr);

    if (enabled) CPU::Interrupt::enable();
}

void Alarm::udelay(Microsecond us) { Alarm::at(Timer::us() + us); }

void Alarm::onTick() {
    int core   = CPU::id();
    List &list = delays_[core];

    Link *head = list.head();
    while (head && elapsed(head->criterion())) {
        list.remove();
        Thread::wakeup(&head->value());
        head = list.head();
    }
}

} // namespace DEPOS
