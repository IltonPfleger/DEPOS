export module Alarm;
import Logger;
import Memory;
import Settings;
import Semaphore;

struct Entry {
    Semaphore semaphore;
    unsigned long value;
    Entry *next;
};

Entry *alarms = nullptr;

export struct Alarm {
    static void delay(unsigned long value) {
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

    static void timer_handler() {
        if (alarms && --alarms->value == 0) {
            Semaphore::v(&alarms->semaphore);
            alarms = alarms->next;
        }
    }
};
