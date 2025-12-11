#pragma once
#include <Traits.hpp>

class Timer {
    struct Channel {
        unsigned long duration;
        unsigned long current[Traits<Machine>::CPUS];
    };

  public:
    static void handler(unsigned long);
    static void init();

  private:
    static inline Channel s_scheduler;
};
