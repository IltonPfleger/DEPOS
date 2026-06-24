#include <Alarm.hpp>
#include <Spin.hpp>
#include <architecture/Timer.hpp>
#include <utility/Console.hpp>

namespace QUARK {

bool Alarm::elapsed(Microsecond us) { return static_cast<intmax_t>(Timer::now() - us) >= 0; }

Alarm::Alarm(Microsecond us)
    : node_(this, us) {
    Spin lock;

    CPU::IRQ::Guard _;

    Alarms &alarms = alarms_[CPU::id()];

    alarms.insert(&node_);

    Thread::sleep(&thread_, &lock);
}

void Alarm::handler() {
    Alarms &alarms = alarms_[CPU::id()];

    while (1) {
        Node *head = alarms.remove();

        if (!head) break;

        if (!elapsed(head->criterion)) {
            alarms.insert(head);
            break;
        }

        Thread::wakeup(&head->value->thread_);
    }
}

} // namespace QUARK
