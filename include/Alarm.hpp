#include <Memory.hpp>
#include <Meta.hpp>
#include <Scheduler/Semaphore.hpp>
#include <Settings.hpp>

struct Alarm {
    struct Entry {
        RawSemaphore semaphore;
        unsigned long value;
        Entry *next;
    };

    static inline Entry *alarms = nullptr;

    template <typename T = void>
    static typename Meta::IF<Settings::Timer::Enable::ALARM, T>::Type delay(unsigned long value) {
        unsigned long time = value * Settings::Timer::ALARM;
        Entry alarm{RawSemaphore(0), time, nullptr};

        if (alarms == nullptr) {
            alarm.next = nullptr;
            alarms     = &alarm;
        } else if (alarm.value < alarms->value) {
            alarms->value -= alarm.value;
            alarm.next = alarms;
            alarms     = &alarm;
        } else {
            unsigned long sum = alarms->value;
            Entry *current    = alarms;
            Entry *previous   = nullptr;
            while (current->next && sum + current->next->value < alarm.value) {
                current = current->next;
                sum += current->value;
            }
            if (current->next == nullptr) {
                alarm.value -= sum;
                current->next = &alarm;
            } else {
                alarm.next = current->next;
                alarm.value -= current->value;
                alarm.next->value -= alarm.value;
                previous->next = &alarm;
            }
        }

        alarm.semaphore.p();
    }

    static void handler() {
        if (alarms && --alarms->value == 0) {
            alarms->semaphore.v();
            alarms = alarms->next;
        }
    }
};
