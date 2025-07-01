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

Entry *entries = nullptr;

export struct Alarm {
    static void delay(unsigned long value) {
        Entry *entry = reinterpret_cast<Entry *>(Memory::malloc(sizeof(Entry), Memory::SYSTEM));
        entry->value = value * Settings::Timer::ALARM;

        entry->next = nullptr;
        Semaphore::create(&entry->semaphore, 0);

        entries = entry;
        Semaphore::p(&entry->semaphore);
        Memory::free(entry, Memory::SYSTEM);
    }

    static void timer_handler() {
        if (entries && --entries->value == 0) {
            //Logger::log("%d\n", entries->value);
            Semaphore::v(&entries->semaphore);
            entries = entries->next;
        }
    }
};
