#pragma once
#include <machine/Traits.hpp>

class Timer {
    struct Channel {
        unsigned long duration;
        unsigned long current[Traits<CPUS>::COUNT];
    };

  public:
    static void handler(unsigned long);
    static void init();

  private:
    static inline Channel scheduler_s;
};
