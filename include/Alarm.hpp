#pragma once

#include <Spin.hpp>
#include <Thread.hpp>

class Alarm {
    struct Delay {
        Thread::Queue queue;
        unsigned long ticks;
        Delay *next;
    };

  public:
    static void udelay(unsigned int);
    static void handler();

  private:
    static inline Delay *s_delays = nullptr;
    static inline Spin s_spin;
};
