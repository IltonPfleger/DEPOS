export module Alarm;
import Logger;
import Memory;
import Semaphore;

struct Entry {
    Semaphore semaphore;
    unsigned int value;
    Entry *next;
};

Entry *entries = nullptr;

export struct Alarm {
    static void delay(unsigned int value) {
        Entry *entry = reinterpret_cast<Entry *>(Memory::malloc(sizeof(Entry), Memory::SYSTEM));

        entry->value = value;
        Semaphore::create(&entry->semaphore, 0);
        Semaphore::p(&entry->semaphore);
        Memory::free(entry, Memory::SYSTEM);
    }

    static void timer_handler() {
        if (entries == nullptr) return;
        if (--entries->value == 0) {
            Semaphore::v(&entries->semaphore);
            entries = entries->next;
        }
    }
};
