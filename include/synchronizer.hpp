#ifndef __synchronizer_hpp
#define __synchronizer_hpp

#include <thread.hpp>

struct Synchronizer {
    Thread::Queue waiting;
    unsigned int value;

    void p() {
        if (CPU::Atomic::fdec(&value) <= 0) Thread::sleep(&waiting);
    }

    void v() {
        if (CPU::Atomic::fadd(&value) <= 0) Thread::wakeup(&waiting);
    }
};

#endif
