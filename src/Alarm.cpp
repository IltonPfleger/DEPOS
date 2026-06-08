#include <Alarm.hpp>
#include <architecture/Timer.hpp>
#include <utility/Console.hpp>

namespace DEPOS {

bool Alarm::elapsed(Microsecond us) { return static_cast<intmax_t>(Timer::now() - us) >= 0; }

Alarm::Alarm(Microsecond us)
    : node_(Thread::Queue(), us) {
    bool enabled = CPU::Interrupt::disable();
    delays_[CPU::id()].insert(&node_);
    Thread::sleep(&node_.value(), nullptr);
    if (enabled) CPU::Interrupt::enable();
}

void Alarm::onTick() {
    List &list = delays_[CPU::id()];

    Node *head = list.head();
    while (head && elapsed(head->criterion())) {
        list.remove();
        Thread::wakeup(&head->value());
        head = list.head();
    }
}

} // namespace DEPOS
