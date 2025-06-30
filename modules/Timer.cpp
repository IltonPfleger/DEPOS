export module Timer;
import CPU;
import Machine;
import Thread;
import Alarm;
import Logger;

volatile uintptr_t& MTIME    = *reinterpret_cast<volatile uintptr_t*>(Machine::CLINT::ADDR + 0xBFF8);
volatile uintptr_t& MTIMECMP = *reinterpret_cast<volatile uintptr_t*>(Machine::CLINT::ADDR + 0x4000);
// MTIMECMP Por Núcleo

constexpr const bool ALARM                        = true;
constexpr const bool SCHEDULER                    = true;
constexpr const unsigned long INTERRUPT_FREQUENCY = 1'000'000;
constexpr const unsigned long SCHEDULER_FREQUENCY = 1'000'000;
constexpr const unsigned long ALARM_FREQUENCY     = INTERRUPT_FREQUENCY;

struct Channel {
    typedef void (*Handler)(void);
    enum { SCHEDULER, ALARM };
    unsigned int initial;
    unsigned int current;
    void (*handler)();
};

struct Channel CHANNELS[2];

export namespace Timer {
    void reset() {
        uintptr_t now = MTIME;
        MTIMECMP      = now + (Machine::CLINT::CLOCK / INTERRUPT_FREQUENCY);
    }

    void init() {
        if constexpr (SCHEDULER) {
            CHANNELS[Channel::SCHEDULER].handler = Thread::timer_handler;
            CHANNELS[Channel::SCHEDULER].initial = INTERRUPT_FREQUENCY / SCHEDULER_FREQUENCY;
            CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
        }

        if constexpr (ALARM) {
            CHANNELS[Channel::ALARM].handler = Alarm::timer_handler;
            CHANNELS[Channel::ALARM].initial = INTERRUPT_FREQUENCY / ALARM_FREQUENCY;
            CHANNELS[Channel::ALARM].current = CHANNELS[Channel::ALARM].initial;
        }

        reset();
        CPU::Interrupt::Timer::enable();
    }

    void handler() {
        Timer::reset();
        if constexpr (SCHEDULER) {
            if (--CHANNELS[Channel::SCHEDULER].current == 0) {
                CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
                CHANNELS[Channel::SCHEDULER].handler();
            }
        }

        // if constexpr (ALARM) {
        //     CHANNELS[Channel::ALARM].handler();
        // }
    }
};  // namespace Timer
