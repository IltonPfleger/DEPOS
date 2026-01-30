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
    static void delay(unsigned int seconds);
    static void handler();

  private:
    static inline Delay *s_delays = nullptr;
    static inline Spin s_lock;
};
