export module Alarm;
import Logger;
import Memory;
import Settings;
import Semaphore;
import Meta;

struct Entry {
    Semaphore semaphore;
    unsigned long value;
    Entry *next;
};

Entry *alarms = nullptr;
template <bool B>
struct _Alarm {
    template <typename T = void>
    static typename Meta::IF<B, T>::Type delay(unsigned long value) {
        Entry *alarm = reinterpret_cast<Entry *>(Memory::malloc(sizeof(Entry), Memory::SYSTEM));
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

        Semaphore::create(&alarm->semaphore, 0);
        Semaphore::p(&alarm->semaphore);
        Memory::free(alarm, Memory::SYSTEM);
    }

    static void handler() {
        if (alarms && --alarms->value == 0) {
            Semaphore::v(&alarms->semaphore);
            alarms = alarms->next;
        }
    }
};

export using Alarm = _Alarm<Settings::Timer::Enable::ALARM>;
