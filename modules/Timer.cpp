export module Timer;
import CPU;
import Definitions;
import Logger;
import Thread;

struct Channel {
    typedef void (*Handler)(void);
    enum Role { ALARM, SCHEDULER };
    unsigned int initial;
    unsigned int current;
    void (*handler)();
};

constexpr uintptr_t BaseAddr        = Machine::Timer::ADDR;
constexpr uintptr_t TICKS_PER_CYCLE = Machine::Timer::CLOCK / Traits::Timer::FREQUENCY;
volatile inline uintptr_t& MTIME    = *reinterpret_cast<volatile uintptr_t*>(BaseAddr + 0xBFF8);
volatile inline uintptr_t& MTIMECMP = *reinterpret_cast<volatile uintptr_t*>(BaseAddr + 0x4000);
struct Channel CHANNELS[2];

export struct Timer {
    static void reset() {
        uintptr_t now = MTIME;
        MTIMECMP      = now + TICKS_PER_CYCLE;
    }

    static void init() {
        if constexpr (Traits::Timer::Channel::SCHEDULER) {
            CHANNELS[Channel::SCHEDULER].handler = Thread::timer_handler;
            CHANNELS[Channel::SCHEDULER].initial = (Traits::Thread::DURATION * 1e6) / Traits::Timer::FREQUENCY;
            CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
        }

        reset();
        CPU::Interrupt::Timer::enable();
    }

    static void handler() {
        Timer::reset();
        if constexpr (Traits::Timer::Channel::SCHEDULER) {
            if (--CHANNELS[Channel::SCHEDULER].current == 0) {
                CHANNELS[Channel::SCHEDULER].current = CHANNELS[Channel::SCHEDULER].initial;
                CHANNELS[Channel::SCHEDULER].handler();
            }
        }
    }
};
