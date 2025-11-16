#pragma once
#include <Traits.hpp>

class Timer {
    struct Channel {
        unsigned long _default;
        unsigned long _current[Traits<Machine>::CPUS];
    };

    static inline Channel _scheduler;

  public:
    static void handler(unsigned long);
    static void init();
};
