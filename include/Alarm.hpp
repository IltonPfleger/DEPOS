#include <Memory.hpp>
#include <Meta.hpp>
#include <Semaphore.hpp>
#include <Settings.hpp>

struct Alarm {
    struct Delay {
        RawSemaphore semaphore;
        unsigned long value;
        Delay *next;
    };

    static inline Delay *delays = nullptr;

    template <typename T = void>
    static typename Meta::IF<Traits<Alarm>::Enable, T>::Type delay(unsigned long value) {
        unsigned long ticks = value * Traits<Alarm>::Frequency;
        Delay *entry        = new (Memory::SYSTEM) Delay{RawSemaphore(0), ticks, nullptr};

        if (!delays || entry->value < delays->value) {
            if (delays) delays->value -= entry->value;
            entry->next = delays;
            delays      = entry;
        } else {
            unsigned long sum = delays->value;
            Delay *current    = delays;

            while (current->next && sum + current->next->value < entry->value) {
                current = current->next;
                sum += current->value;
            }

            entry->value -= sum;

            if (current->next) current->next->value -= entry->value;

            entry->next   = current->next;
            current->next = entry;
        }

        entry->semaphore.p();
        delete entry;
    }

    static void handler() {
        if (delays && --delays->value == 0) {
            delays->semaphore.v();
            delays = delays->next;
        }
    }
};
