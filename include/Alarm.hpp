#include <Meta.hpp>
#include <Semaphore.hpp>
#include <Settings.hpp>

struct Entry {
    Semaphore semaphore;
    unsigned long value;
    Entry *next;
};

Entry *alarms = nullptr;
struct Alarm {
    template <typename T = void>
    static typename Meta::IF<Settings::Timer::Enable::ALARM, T>::Type delay(unsigned long value) {
        Entry *alarm = new (Memory::SYSTEM) Entry;
        alarm->value = value * Settings::Timer::ALARM;

        if (alarms == nullptr) {
            alarm->next = nullptr;
            alarms      = alarm;
        } else if (alarm->value < alarms->value) {
            alarms->value -= alarm->value;
            alarm->next = alarms;
            alarms      = alarm;
        } else {
            unsigned long sum = alarms->value;
            Entry *current    = alarms;
            while (current->next && sum + current->next->value < alarm->value) {
                current = current->next;
                sum += current->value;
            }
            if (current->next == nullptr) {
                alarm->value -= sum;
                current->next = alarm;
            } else {
                alarm->next = current->next;
                alarm->value -= current->value;
                alarm->next->value -= alarm->value;
            }
        }

		new(&alarm->semaphore) Semaphore(0);
		alarm->semaphore.p();
        delete alarm;
    }

    static void handler() {
        if (alarms && --alarms->value == 0) {
			alarms->semaphore.v();
            alarms = alarms->next;
        }
    }
};
