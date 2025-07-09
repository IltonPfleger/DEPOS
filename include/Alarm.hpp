#include <IO/Logger.hpp>
#include <Memory.hpp>
#include <Meta.hpp>
#include <Semaphore.hpp>
#include <Traits.hpp>

struct Alarm {
    struct Delay {
        RawSemaphore semaphore;
        unsigned long value;
        Delay *next;
    };

    static inline Delay *delays = nullptr;

    template <typename T = void>
        requires(Traits<Alarm>::Enable && Traits<Alarm>::Frequency >= 1'000'000)
    static void udelay(unsigned long useconds) {
        CPU::Interrupt::disable();
        unsigned long ticks = useconds * (Traits<Alarm>::Frequency / 1'000'000);
        Delay entry{RawSemaphore(0), ticks, nullptr};

        if (!delays || entry.value < delays->value) {
            if (delays) delays->value -= entry.value;
            entry.next = delays;
            delays     = &entry;
        } else {
            unsigned long sum = delays->value;
            Delay *current    = delays;

            while (current->next && sum + current->next->value < entry.value) {
                current = current->next;
                sum += current->value;
            }

            entry.value -= sum;

            if (current->next) current->next->value -= entry.value;

            entry.next    = current->next;
            current->next = &entry;
        }
        entry.semaphore.p();
        delays = delays->next;
    }

    template <typename T = void>
        requires(Traits<Alarm>::Enable)
    static void delay(unsigned long seconds) {
        CPU::Interrupt::disable();
        unsigned long ticks = seconds * Traits<Alarm>::Frequency;
        Delay entry{RawSemaphore(0), ticks, nullptr};

        if (!delays || entry.value < delays->value) {
            if (delays) delays->value -= entry.value;
            entry.next = delays;
            delays     = &entry;
        } else {
            unsigned long sum = delays->value;
            Delay *current    = delays;

            while (current->next && sum + current->next->value < entry.value) {
                current = current->next;
                sum += current->value;
            }

            entry.value -= sum;

            if (current->next) current->next->value -= entry.value;

            entry.next    = current->next;
            current->next = &entry;
        }
        entry.semaphore.p();
        delays = delays->next;
    }

    static void handler() {
        if (delays) {
            if (delays->value > 0) {
                delays->value--;
            } else if (delays->value == 0) {
                delays->semaphore.v();
            } else {
                delays = delays->next;
            }
        }
    }
};
