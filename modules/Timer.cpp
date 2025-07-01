export module Timer;
import CPU;
import Machine;
import Settings;
import Thread;
import Alarm;
import Logger;

volatile uintptr_t& MTIME    = *reinterpret_cast<volatile uintptr_t*>(Machine::CLINT::ADDR + 0xBFF8);
volatile uintptr_t& MTIMECMP = *reinterpret_cast<volatile uintptr_t*>(Machine::CLINT::ADDR + 0x4000);

struct Channel {
    typedef void (*Handler)(void);
    enum { SCHEDULER, ALARM };
    uintptr_t initial;
    uintptr_t current;
    void (*handler)();
};

struct Channel CHANNELS[2];

export namespace Timer {
    void reset() {
        uintptr_t now = MTIME;
        MTIMECMP      = now + (Machine::CLINT::CLOCK / Settings::Timer::FREQUENCY);
    }

    void init() {
        if constexpr (Settings::Timer::Enable::SCHEDULER) {
            CHANNELS[Channel::SCHEDULER].handler = Thread::timer_handler;
            CHANNELS[Channel::SCHEDULER].initial = Settings::Timer::FREQUENCY / Settings::Timer::SCHEDULER;
            CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
        }

        if constexpr (Settings::Timer::Enable::ALARM) {
            CHANNELS[Channel::ALARM].handler = Alarm::handler;
            CHANNELS[Channel::ALARM].initial = Settings::Timer::FREQUENCY / Settings::Timer::ALARM;
            CHANNELS[Channel::ALARM].current = CHANNELS[Channel::ALARM].initial;
        }

        reset();
        CPU::Interrupt::Timer::enable();
    }

    void handler() {
        reset();
        if constexpr (Settings::Timer::Enable::SCHEDULER) {
            if (--(CHANNELS[Channel::SCHEDULER].current) == 0) {
                CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
                CHANNELS[Channel::SCHEDULER].handler();
            }
        }

        if constexpr (Settings::Timer::Enable::ALARM) {
            if (--(CHANNELS[Channel::ALARM].current) <= 0) {
                CHANNELS[Channel::ALARM].current = CHANNELS[Channel::ALARM].initial;
                CHANNELS[Channel::ALARM].handler();
            }
        }
    }
};  // namespace Timer
